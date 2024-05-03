cbuffer externalData : register(b0)
{
    uint3 gridsize;
}

Texture3D pressure : register(t0);
Texture3D velocityIn : register(t1);
RWTexture3D<float3> velocityOut : register(u0);

SamplerState standardSampler : register(s0);

[numthreads(8, 8, 8)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 fID = float3(DTid);
    
    float pressR = pressure.SampleLevel(standardSampler, (fID + float3(1, 0, 0)) / gridsize, 0).x;
    float pressL = pressure.SampleLevel(standardSampler, (fID + float3(-1, 0, 0)) / gridsize, 0).x;
    float pressU = pressure.SampleLevel(standardSampler, (fID + float3(0, 1, 0)) / gridsize, 0).x;
    float pressD = pressure.SampleLevel(standardSampler, (fID + float3(0, -1, 0)) / gridsize, 0).x;
    float pressF = pressure.SampleLevel(standardSampler, (fID + float3(0, 0, 1)) / gridsize, 0).x;
    float pressB = pressure.SampleLevel(standardSampler, (fID + float3(0, 0, -1)) / gridsize, 0).x;
    
    float3 pressureGradient = 0.5f * float3(pressR - pressL, pressU - pressD, pressF - pressB);
    
    velocityOut[DTid] = velocityIn[DTid].xyz - pressureGradient;
}