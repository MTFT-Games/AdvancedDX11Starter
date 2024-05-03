cbuffer externalData : register(b0)
{
    uint3 gridsize;
}

Texture3D divergence : register(t0);
Texture3D pressureIn : register(t1);
RWTexture3D<float> pressureOut : register(u0);

SamplerState standardSampler : register(s0);

[numthreads(8, 8, 8)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 fID = float3(DTid);
    
    float pressR = pressureIn.SampleLevel(standardSampler, (fID + float3(1, 0, 0)) / gridsize, 0).x;
    float pressL = pressureIn.SampleLevel(standardSampler, (fID + float3(-1, 0, 0)) / gridsize, 0).x;
    float pressU = pressureIn.SampleLevel(standardSampler, (fID + float3(0, 1, 0)) / gridsize, 0).x;
    float pressD = pressureIn.SampleLevel(standardSampler, (fID + float3(0, -1, 0)) / gridsize, 0).x;
    float pressF = pressureIn.SampleLevel(standardSampler, (fID + float3(0, 0, 1)) / gridsize, 0).x;
    float pressB = pressureIn.SampleLevel(standardSampler, (fID + float3(0, 0, -1)) / gridsize, 0).x;
    
    pressureOut[DTid] = 
        (pressL + pressR + pressD + pressU + pressB + pressF - divergence[DTid].x) / 6.0f;

}