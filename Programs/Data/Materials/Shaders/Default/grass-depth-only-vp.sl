#include "common.slh"

vertex_in
{
    float3 position : POSITION;
    float2 uv0 : TEXCOORD0;
    float3 uv1 : TEXCOORD1;
    float3 uv2 : TEXCOORD2;
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 projectedPosition : TEXCOORD0;
};

uniform sampler2D heightmap;
uniform sampler2D vegetationmap;

[auto][instance] property float4x4 worldViewProjMatrix;
[auto][a] property float heightmapTextureSize;

[material][a] property float3 tilePos;
[material][a] property float3 worldSize;
[material][a] property float2 lodSwitchScale;
[material][a] property float4 vegWaveOffsetx;
[material][a] property float4 vegWaveOffsety;
//8 floats: xxxxyyyy (xy per layer)

vertex_out vp_main(vertex_in input)
{
    float3 inPosition = input.position.xyz;
    float2 inTexCoord0 = input.uv0;
    float3 inTexCoord1 = input.uv1;
    float3 inTexCoord2 = input.uv2;
    
    float3 clusterCenter = float3(inTexCoord2.x + tilePos.x, inTexCoord2.y + tilePos.y, inTexCoord2.z);
    
    float2 uv = 0.5 - clusterCenter.xy / worldSize.xy;
    float2 uvColor = float2(1.0 - uv.x, uv.y);
    float2 uvHeight = float2(uvColor.x, 1.0 - uv.y) + 0.5 / heightmapTextureSize;

    #if HEIGHTMAP_FLOAT_TEXTURE
        float heightSample = tex2Dlod(heightmap, uvHeight, 0.0).r;
    #else
        float4 heightVec = tex2Dlod(heightmap, uvHeight, 0.0);
        float heightSample = dot(heightVec, float4(0.00022888532845, 0.00366216525521, 0.05859464408331, 0.93751430533303));
    #endif

    float height = heightSample * worldSize.z;

    float3 pos = float3(inPosition.x + tilePos.x, inPosition.y + tilePos.y, inPosition.z);
    pos.z += height;
    clusterCenter.z += height;

    float clusterScale = tilePos.z;
    if (int(inTexCoord1.x) == int(lodSwitchScale.x))
    {
        clusterScale *= lodSwitchScale.y;
    }

    float4 vegetationMask = tex2Dlod(vegetationmap, uvColor, 0.0);

    // wave transform
    int waveIndex = int(inTexCoord1.y);
    pos.x += inTexCoord1.z * vegWaveOffsetx[waveIndex];
    pos.y += inTexCoord1.z * vegWaveOffsety[waveIndex];
    pos = lerp(clusterCenter, pos, vegetationMask.a * clusterScale);

    vertex_out output;
    output.projectedPosition = mul(float4(pos, 1.0), worldViewProjMatrix);
    output.position = output.projectedPosition;
    return output;
}
