cbuffer externalData : register(b0)
{
    float deltaTime;
    uint3 gridsize;
    uint channels;
}

Texture3D velocities : register(t0);
Texture3D input : register(t1);

RWTexture3D<float> out1 : register(u0);
RWTexture3D<float3> out3 : register(u1);

SamplerState standardSampler : register(s0);

[numthreads(8, 8, 8)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 currentIndex = float3(DTid);
    float3 velocity = velocities[DTid].xyz;
    float3 inputIndex = currentIndex - velocity * deltaTime;
    
    
    if (channels == 3)
    {
        out3[DTid] = input.SampleLevel(standardSampler, inputIndex/gridsize, 0).rgb;
    }
    else
    {
        out1[DTid] = input.SampleLevel(standardSampler, inputIndex / gridsize, 0).r;
    }
}