#include "common.slh"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
    [lowp] half3 vegetationColor :COLOR0;
};

fragment_out
{
    float4 gbufferDepth : SV_TARGET0;
    float4 gbuffer0 : SV_TARGET1;
    float4 gbuffer1 : SV_TARGET2;
    float4 gbuffer2 : SV_TARGET3;
};

uniform sampler2D albedo;

#if LOD_COLOR
    [material][instance] property float3 lodColor = float3(1, 1, 1);
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float4 textureColor0 = tex2D(albedo, input.texCoord);
    float3 color = textureColor0.rgb * float3(input.vegetationColor) * 2.0;

    #if LOD_COLOR
        color += lodColor;
    #endif

    output.gbuffer0 = float4(color, 0.0);
    output.gbuffer1 = 0.0;
    output.gbuffer2 = 0.0;
    output.gbufferDepth.r = input.projectedPosition.z / input.projectedPosition.w * ndcToZMappingScale + ndcToZMappingOffset;
    output.gbufferDepth.gba = 0.0;

    return output;
}
