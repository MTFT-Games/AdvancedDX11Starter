#include "Emitter.h"

Emitter::Emitter(
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	float lifetime, 
	float particlesPerSecond1, 
	float particlesPerSecond2, 
	int maxParticles,
	DirectX::XMFLOAT3 startPos1, 
	DirectX::XMFLOAT3 startPos2, 
	DirectX::XMFLOAT3 startVel1,
	DirectX::XMFLOAT3 startVel2,
	DirectX::XMFLOAT3 accel1,
	DirectX::XMFLOAT3 accel2,
	DirectX::XMFLOAT4 startColor,
	DirectX::XMFLOAT4 endColor,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture, 
	std::shared_ptr<SimplePixelShader> ps, 
	std::shared_ptr<SimpleVertexShader> vs, 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState) :
	lifetime(lifetime), 
	emitDelay1(1.0f/particlesPerSecond1), 
	emitDelay2(1.0f/particlesPerSecond2), 
	emitDelay(emitDelay1),
	maxParticles(maxParticles), 
	startPos1(startPos1),
	startPos2(startPos2),
	startVel1(startVel1),
	startVel2(startVel2),
	accel1(accel1),
	accel2(accel2),
	startColor(startColor),
	endColor(endColor),
	texture(texture),
	ps(ps),
	vs(vs),
	sampler(sampler),
	blendState(blendState)
{
	// Make a dynamic buffer to hold all particle data on GPU
	// Note: We'll be overwriting this every frame with new lifetime data
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(Particle);
	desc.ByteWidth = sizeof(Particle) * maxParticles;
	device->CreateBuffer(&desc, 0, particleBuffer.GetAddressOf());
	
	// Create an SRV that points to a structured buffer of particles
	// so we can grab this data in a vertex shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	device->CreateShaderResourceView(particleBuffer.Get(), &srvDesc, particleSRV.GetAddressOf());

	// make index buff
	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}

	// Create the index buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * (UINT)(maxParticles * 6); // Number of indices
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices;
	device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());

	delete[] indices;

	particles = new Particle[maxParticles];
}

Emitter::~Emitter()
{
	delete[] particles;
}

void Emitter::Update(float totalTime, float deltaTime)
{
	// Kill particles that gett to old
	// Just check if the first living particle is too old. If not, the rest wont be
	while (livingCount > 0 && particles[firstLivingIndex].EmitTime + lifetime < totalTime)
	{
		firstLivingIndex++;
		firstLivingIndex %= maxParticles;
		livingCount--;
	}

	// Emit
	timeSinceEmit += deltaTime;
	while (timeSinceEmit > emitDelay && livingCount < maxParticles)
	{
		particles[firstDeadIndex].EmitTime = totalTime;
		particles[firstDeadIndex].StartPos.x = startPos1.x + rand() / (float)RAND_MAX * (startPos2.x - startPos1.x);
		particles[firstDeadIndex].StartPos.y = startPos1.y + rand() / (float)RAND_MAX * (startPos2.y - startPos1.y);
		particles[firstDeadIndex].StartPos.z = startPos1.z + rand() / (float)RAND_MAX * (startPos2.z - startPos1.z);
		particles[firstDeadIndex].StartVel.x = startVel1.x + rand() / (float)RAND_MAX * (startVel2.x - startVel1.x);
		particles[firstDeadIndex].StartVel.y = startVel1.y + rand() / (float)RAND_MAX * (startVel2.y - startVel1.y);
		particles[firstDeadIndex].StartVel.z = startVel1.z + rand() / (float)RAND_MAX * (startVel2.z - startVel1.z);
		particles[firstDeadIndex].Accel.x = accel1.x + rand() / (float)RAND_MAX * (accel2.x - accel1.x);
		particles[firstDeadIndex].Accel.y = accel1.y + rand() / (float)RAND_MAX * (accel2.y - accel1.y);
		particles[firstDeadIndex].Accel.z = accel1.z + rand() / (float)RAND_MAX * (accel2.z - accel1.z);
		particles[firstDeadIndex].StartColor = startColor;
		particles[firstDeadIndex].EndColor = endColor;

		firstDeadIndex++;
		firstDeadIndex %= maxParticles;
		livingCount++;

		timeSinceEmit -= emitDelay;
		if (emitDelay1 != emitDelay2)
		{
			emitDelay = emitDelay1 + rand() / (float)RAND_MAX * (emitDelay2 - emitDelay1);
		}
	}
}

void Emitter::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, float totalTime, std::shared_ptr<Camera> camera)
{
	// Map the buffer, locking it on the GPU so we can write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	// How are living particles arranged in the buffer?
	if (firstLivingIndex < firstDeadIndex)
	{
		// Only copy from FirstAlive -> FirstDead
		memcpy(
			mapped.pData, // Destination = start of particle buffer
			particles + firstLivingIndex, // Source = particle array, offset to first living particle
			sizeof(Particle) * livingCount); // Amount = number of particles (measured in BYTES!)
	}
	else
	{
		// Copy from 0 -> FirstDead
		memcpy(
			mapped.pData, // Destination = start of particle buffer
			particles, // Source = start of particle array
			sizeof(Particle) * firstDeadIndex); // Amount = particles up to first dead (measured in BYTES!)
		
		// ALSO copy from FirstAlive -> End
		memcpy(
			(void*)((Particle*)mapped.pData + firstDeadIndex), // Destination = particle buffer, AFTER the data we copied in previous memcpy()
			particles + firstLivingIndex, // Source = particle array, offset to first living particle
			sizeof(Particle) * (maxParticles - firstLivingIndex)); // Amount = number of living particles at end of array (measured in BYTES!)
	}

	// Unmap (unlock) now that we're done with it
	context->Unmap(particleBuffer.Get(), 0);

	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Turn on these shaders
	vs->SetShader();
	ps->SetShader();

	// Send data to the vertex shader
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->SetFloat("currentTime", totalTime);
	vs->SetFloat("lifetime", lifetime);
	vs->CopyAllBufferData();

	vs->SetShaderResourceView("ParticleData", particleSRV);
	ps->SetShaderResourceView("Texture", texture);
	ps->SetSamplerState("BasicSampler", sampler);

	context->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);

	context->DrawIndexed(livingCount * 6, 0, 0);
}
