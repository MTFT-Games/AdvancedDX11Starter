// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
    matrix view;
    matrix projection;
    float currentTime;
    float lifetime;
};

struct Particle
{
    float EmitTime;
    float3 StartPos;
    float4 StartColor;
    float4 EndColor;
    float3 StartVel;
    float3 Accel;
};

// Out of the vertex shader (and eventually input to the PS)
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

// Buffer of particle data
StructuredBuffer<Particle> ParticleData : register(t0);

VertexToPixel main(uint id : SV_VertexID)
{
    // Get particle
    uint particleID = id / 4;
    uint cornerID = id % 4;
    Particle p = ParticleData.Load(particleID);
    
    // Simulate (just gravity for now)
    float age = currentTime - p.EmitTime;
    float lifePercent = age / lifetime;
    // position + velocity * time + 0.5 * acceleration * time^2 | for constant accel
    float3 position = p.StartPos + p.StartVel * age + p.Accel * age * age * 0.5;
    
    // billboarding (I think theres a better? way to do this like the fill screen vertex shader from ggp)
    float2 offsets[4];
    offsets[0] = float2(-1.0f, +1.0f);
    offsets[1] = float2(+1.0f, +1.0f);
    offsets[2] = float2(+1.0f, -1.0f);
    offsets[3] = float2(-1.0f, -1.0f);
    position += float3(view._11, view._12, view._13) * offsets[cornerID].x;
    position += float3(view._21, view._22, view._23) * offsets[cornerID].y;
    
    float2 uvs[4];
    uvs[0] = float2(0, 0);
    uvs[1] = float2(1, 0);
    uvs[2] = float2(1, 1);
    uvs[3] = float2(0, 1);
    
    VertexToPixel output;
    output.uv = uvs[cornerID];
    output.screenPosition = mul(mul(projection, view), float4(position, 1.0));
    output.color = lerp(p.StartColor, p.EndColor, lifePercent);
    
	return output;
}