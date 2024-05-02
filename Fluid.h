#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
#include "Transform.h"
#include <memory>
#include "SimpleShader.h"

struct FluidDataBuffer
{
	unsigned int channels;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
};


class Fluid
{
public:
	Fluid( // TODO: need to take and store a clamping sampler. and a zeroing border one. advection uses clamp and linear interp i think, divergence and pressure uses 0 and i think no interpolation. tinker tho
		Microsoft::WRL::ComPtr<ID3D11Device> device, 
		std::shared_ptr<SimplePixelShader> ps, 
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimpleComputeShader> advectionCS,
		std::shared_ptr<SimpleComputeShader> divergenceCS,
		std::shared_ptr<SimpleComputeShader> pressureCS,
		std::shared_ptr<SimpleComputeShader> projectionCS);
	void Update();
	void Draw();
	Transform transform;
private:
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimpleVertexShader> vs;

	std::shared_ptr<SimpleComputeShader> advectionCS;
	std::shared_ptr<SimpleComputeShader> divergenceCS;
	std::shared_ptr<SimpleComputeShader> pressureCS;
	std::shared_ptr<SimpleComputeShader> projectionCS;

	void Advect(FluidDataBuffer buffers[2]); // remember to swap them when implementing

	DirectX::XMUINT3 gridSize;
	
	FluidDataBuffer velocities[2];
	FluidDataBuffer pressures[2];
	FluidDataBuffer densities[2];
	FluidDataBuffer divergence;
	// TODO temp and vorticity
};

