#include "common.slh"
#include "blending.slh"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;

    #if FLOWMAP
        float3 varFlowData : TEXCOORD4;
    #endif
};

fragment_out
{
    float4 gbufferDepth : SV_TARGET0;
    float4 gbuffer0 : SV_TARGET1;
    float4 gbuffer1 : SV_TARGET2;
    float4 gbuffer2 : SV_TARGET3;
};

uniform sampler2D albedo;

#if FLOWMAP
    uniform sampler2D flowmap;
#endif

#if FLATCOLOR
    [material][a] property float4 flatColor = float4(0, 0, 0, 0);
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    #if FLOWMAP
        float2 flowtc = input.varTexCoord0;
        float3 flowData = input.varFlowData;
        float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;
        half3 flowSample1 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.x).rgb);
        half3 flowSample2 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.y).rgb);
        half3 textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z));
    #else
        half3 textureColor0 = half3(tex2D(albedo, input.varTexCoord0).rgb);
    #endif

    float3 color = float3(textureColor0.rgb);

    #if FLATCOLOR
        color *= flatColor.rgb;
    #endif

    output.gbuffer0 = float4(color, 0.5);
    output.gbuffer1 = 0.0;
    output.gbuffer2 = 0.0;
    output.gbufferDepth.r = 1.0;
    output.gbufferDepth.gba = 0.0;

    return output;
}
