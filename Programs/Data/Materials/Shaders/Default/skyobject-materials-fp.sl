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
    float4 color : SV_TARGET0;
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
        float2 flowDir = float2(tex2D(flowmap, flowtc ).xy) * 2.0 - 1.0;
        half3 flowSample1 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.x).rgb);
        half3 flowSample2 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.y).rgb);
        half3 textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z));
    #else
        half3 textureColor0 = half3(tex2D(albedo, input.varTexCoord0).rgb);
    #endif

    float3 color = float3(textureColor0.rgb);

    output.color = float4(color.rgb, 1.0);

    #if FLATCOLOR
        output.color *= flatColor;
    #endif

    return output;
}
