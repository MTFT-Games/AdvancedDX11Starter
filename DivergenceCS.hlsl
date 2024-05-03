cbuffer externalData : register(b0)
{
    uint3 gridsize;
}

Texture3D velocities : register(t0);
RWTexture3D<float> divergences : register(u0);

SamplerState standardSampler : register(s0);

[numthreads(8, 8, 8)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 fID = float3(DTid);
    
    float velR = velocities.SampleLevel(standardSampler, (fID + float3(1, 0, 0))/gridsize, 0).x;
    float velL = velocities.SampleLevel(standardSampler, (fID + float3(-1, 0, 0)) / gridsize, 0).x;
    float velU = velocities.SampleLevel(standardSampler, (fID + float3(0, 1, 0)) / gridsize, 0).y;
    float velD = velocities.SampleLevel(standardSampler, (fID + float3(0, -1, 0)) / gridsize, 0).y;
    float velF = velocities.SampleLevel(standardSampler, (fID + float3(0, 0, 1)) / gridsize, 0).z;
    float velB = velocities.SampleLevel(standardSampler, (fID + float3(0, 0, -1)) / gridsize, 0).z;
    
    // Not sure why we half this, its in the slides tho. 
    // Might be in the GPU gems but i only skimmed that 
    // due to time and my brain melting
    divergences[DTid] = 0.5f * (
        (velR - velL) +
        (velU - velD) +
        (velF - velB));
}