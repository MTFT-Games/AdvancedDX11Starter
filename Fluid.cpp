#include "Fluid.h"
using namespace DirectX;

Fluid::Fluid(
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	std::shared_ptr<SimplePixelShader> ps, 
	std::shared_ptr<SimpleVertexShader> vs, 
	std::shared_ptr<SimpleComputeShader> advectionCS, 
	std::shared_ptr<SimpleComputeShader> divergenceCS, 
	std::shared_ptr<SimpleComputeShader> pressureCS, 
	std::shared_ptr<SimpleComputeShader> projectionCS) :
	ps(ps),
	vs(vs),
	advectionCS(advectionCS),
	divergenceCS(divergenceCS),
	pressureCS(pressureCS),
	projectionCS(projectionCS),
	gridSize(XMUINT3(64, 64, 64))
{
	D3D11_TEXTURE3D_DESC desc = {};
	desc.Width = gridSize.x;
	desc.Height = gridSize.y;
	desc.Depth = gridSize.z;
	desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture3D> texture;
	device->CreateTexture3D(&desc, 0, texture.GetAddressOf());
	velocities[0].channels = 3;
	device->CreateShaderResourceView(texture.Get(), 0, velocities[0].SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, velocities[0].UAV.GetAddressOf());

	device->CreateTexture3D(&desc, 0, texture.ReleaseAndGetAddressOf());
	velocities[1].channels = 3;
	device->CreateShaderResourceView(texture.Get(), 0, velocities[1].SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, velocities[1].UAV.GetAddressOf());

	desc.Format = DXGI_FORMAT_R32_FLOAT;
	device->CreateTexture3D(&desc, 0, texture.ReleaseAndGetAddressOf());
	pressures[0].channels = 1;
	device->CreateShaderResourceView(texture.Get(), 0, pressures[0].SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, pressures[0].UAV.GetAddressOf());

	device->CreateTexture3D(&desc, 0, texture.ReleaseAndGetAddressOf());
	pressures[1].channels = 1;
	device->CreateShaderResourceView(texture.Get(), 0, pressures[1].SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, pressures[1].UAV.GetAddressOf());

	device->CreateTexture3D(&desc, 0, texture.ReleaseAndGetAddressOf());
	densities[0].channels = 1;
	device->CreateShaderResourceView(texture.Get(), 0, densities[0].SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, densities[0].UAV.GetAddressOf());

	device->CreateTexture3D(&desc, 0, texture.ReleaseAndGetAddressOf());
	densities[1].channels = 1;
	device->CreateShaderResourceView(texture.Get(), 0, densities[1].SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, densities[1].UAV.GetAddressOf());

	device->CreateTexture3D(&desc, 0, texture.ReleaseAndGetAddressOf());
	divergence.channels = 1;
	device->CreateShaderResourceView(texture.Get(), 0, divergence.SRV.GetAddressOf());
	device->CreateUnorderedAccessView(texture.Get(), 0, divergence.UAV.GetAddressOf());
}

void Fluid::Update(float deltaTime)
{
	Advect(velocities, deltaTime);
	Advect(densities, deltaTime);
	// I dont think anything else needs advection yet

	Diverge();
	for (size_t i = 0; i < 16; i++)
	{
		Pressure();
	}
	Project();
	// I think thats it...? this feels like a short update for the complicated thing that this is
}

void Fluid::Advect(FluidDataBuffer buffers[2], float deltaTime)
{
	// Need to set srv and uavs
	advectionCS->SetShader();
	advectionCS->SetShaderResourceView("velocities", velocities->SRV);
	advectionCS->SetShaderResourceView("input", buffers[0].SRV);
	if (buffers[1].channels == 3)
	{
		advectionCS->SetUnorderedAccessView("out3", buffers[1].UAV);
	}
	else
	{
		advectionCS->SetUnorderedAccessView("out1", buffers[1].UAV);
	}

	// sampler needed too
	advectionCS->SetSamplerState("standardSampler", clampingLinSampler);

	// set constant buffers
	advectionCS->SetFloat("delatTime", deltaTime);
	// there isnt a set uint :(. i think this might work. I could also just use an int instead but naaa
	advectionCS->SetData("gridsize", &gridSize, sizeof(XMUINT3));
	advectionCS->SetData("channels", &buffers[1].channels, sizeof(unsigned int));

	// REMEMBER TO copy all buffers
	advectionCS->CopyAllBufferData();

	// dispatch
	advectionCS->DispatchByThreads(gridSize.x, gridSize.y, gridSize.z);

	// swap buffers I think this will probably work 
	FluidDataBuffer temp = buffers[0];
	buffers[0] = buffers[1];
	buffers[1] = temp;

	// uhhhhhhh *checks demo* is there anything else?
	// oh i should probably unset buffers, that seems like a good idea
	advectionCS->SetShaderResourceView("velocities", 0);
	advectionCS->SetShaderResourceView("input", 0);
	if (buffers[1].channels == 3)
	{
		advectionCS->SetUnorderedAccessView("out3", 0);
	}
	else
	{
		advectionCS->SetUnorderedAccessView("out1", 0);
	}
}

void Fluid::Diverge()
{
	divergenceCS->SetShader();
	divergenceCS->SetShaderResourceView("velocities", velocities[0].SRV);
	divergenceCS->SetUnorderedAccessView("divergences", divergence.UAV);
	divergenceCS->SetSamplerState("standardSampler", borderBlackSampler);
	divergenceCS->SetData("gridsize", &gridSize, sizeof(XMUINT3));
	divergenceCS->CopyAllBufferData();

	divergenceCS->DispatchByThreads(gridSize.x, gridSize.y, gridSize.z);

	divergenceCS->SetShaderResourceView("velocities", 0);
	divergenceCS->SetUnorderedAccessView("divergences", 0);
}

void Fluid::Pressure()
{
	pressureCS->SetShader();
	pressureCS->SetShaderResourceView("divergence", divergence.SRV);
	pressureCS->SetShaderResourceView("pressureIn", pressures[0].SRV);
	pressureCS->SetUnorderedAccessView("pressureOut", pressures[1].UAV);
	pressureCS->SetSamplerState("standardSampler", borderBlackSampler);
	pressureCS->SetData("gridsize", &gridSize, sizeof(XMUINT3));
	pressureCS->CopyAllBufferData();
	
	pressureCS->DispatchByThreads(gridSize.x, gridSize.y, gridSize.z);
	
	FluidDataBuffer temp = pressures[0];
	pressures[0] = pressures[1];
	pressures[1] = temp;

	pressureCS->SetShaderResourceView("divergence", 0);
	pressureCS->SetShaderResourceView("pressureIn", 0);
	pressureCS->SetUnorderedAccessView("pressureOut", 0);
}

void Fluid::Project()
{
	projectionCS->SetShader();
	projectionCS->SetShaderResourceView("pressure", pressures[0].SRV);
	projectionCS->SetShaderResourceView("velocityIn", velocities[0].SRV);
	projectionCS->SetUnorderedAccessView("velocityOut", velocities[1].UAV);
	projectionCS->SetSamplerState("standardSampler", borderBlackSampler);
	projectionCS->SetData("gridsize", &gridSize, sizeof(XMUINT3));
	projectionCS->CopyAllBufferData();

	projectionCS->DispatchByThreads(gridSize.x, gridSize.y, gridSize.z);

	FluidDataBuffer temp = velocities[0];
	velocities[0] = velocities[1];
	velocities[1] = temp;

	projectionCS->SetShaderResourceView("pressure", 0);
	projectionCS->SetShaderResourceView("velocityIn", 0);
	projectionCS->SetUnorderedAccessView("velocityOut", 0);
}
