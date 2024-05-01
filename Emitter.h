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
	DirectX::XMFLOAT4 StartColor;
	DirectX::XMFLOAT4 EndColor;
	DirectX::XMFLOAT3 StartVel;
	DirectX::XMFLOAT3 Accel;
};

class Emitter
{
public:
	Emitter(
		Microsoft::WRL::ComPtr<ID3D11Device> device, 
		float lifetime, 
		float particlesPerSecond1, // lowest particles per sec,  each particle will take a random amount of time between these values
		float particlesPerSecond2, // highest particles per sec, 
		int maxParticles,
		DirectX::XMFLOAT3 startPos1, // varies same as particles per sec
		DirectX::XMFLOAT3 startPos2, 
		DirectX::XMFLOAT3 startVel1, // same ^^
		DirectX::XMFLOAT3 startVel2, 
		DirectX::XMFLOAT3 accel1, // same ^^
		DirectX::XMFLOAT3 accel2, 
		DirectX::XMFLOAT4 startColor, // lerps to endColor over lifetime
		DirectX::XMFLOAT4 endColor, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture, 
		std::shared_ptr<SimplePixelShader> ps, 
		std::shared_ptr<SimpleVertexShader> vs, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11BlendState> blendState);
	~Emitter();
	void Update(float totalTime, float deltaTime);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, float totalTime, std::shared_ptr<Camera> camera);
	float lifetime;
	float emitDelay1;
	float emitDelay2;
	float emitDelay;
	DirectX::XMFLOAT3 startPos1;
	DirectX::XMFLOAT3 startPos2;
	DirectX::XMFLOAT3 startVel1;
	DirectX::XMFLOAT3 startVel2;
	DirectX::XMFLOAT3 accel1;
	DirectX::XMFLOAT3 accel2;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	// TODO: convinience methods like more constructors, setters, etc
private:
	// TODO: emmit func
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

