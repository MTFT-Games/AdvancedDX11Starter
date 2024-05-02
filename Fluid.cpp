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
