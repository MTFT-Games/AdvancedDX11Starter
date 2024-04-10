#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>
#include "SimpleShader.h"
#include "Camera.h"

struct Particle
{
	float EmitTime;
	DirectX::XMFLOAT3 StartPos;
};

class Emitter
{
public:
	Emitter(
		Microsoft::WRL::ComPtr<ID3D11Device> device, 
		float lifetime, 
		float particlesPerSecond,
		int maxParticles,
		DirectX::XMFLOAT3 startPos, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture, 
		std::shared_ptr<SimplePixelShader> ps, 
		std::shared_ptr<SimpleVertexShader> vs, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	~Emitter();
	void Update(float totalTime, float deltaTime);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, float totalTime, std::shared_ptr<Camera> camera);
	float lifetime;
	float emitDelay;
	DirectX::XMFLOAT3 startPos;
private:
	int maxParticles;
	Particle* particles; 
	int firstDeadIndex = 0;
	int firstLivingIndex = 0;
	int livingCount = 0;
	float timeSinceEmit = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimpleVertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
};

