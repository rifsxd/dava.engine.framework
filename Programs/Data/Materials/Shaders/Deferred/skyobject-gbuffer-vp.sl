#include "common.slh"

vertex_in
{
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;

    #if FLOWMAP
        [lowp] float3 varFlowData : TEXCOORD4; // For flowmap animations - xy next frame uv. z - frame time
    #endif
};

[auto][a] property float4x4 worldViewProjMatrix;

#if FLOWMAP
    [auto][a] property float globalTime;
    [material][a] property float flowAnimSpeed = 0;
    [material][a] property float flowAnimOffset = 0;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    #if FLOWMAP
        float flowSpeed = flowAnimSpeed;
        float flowOffset = flowAnimOffset;

        float scaledTime = globalTime * flowSpeed;
        float2 flowPhases = frac(float2(scaledTime, scaledTime + 0.5)) - float2(0.5, 0.5);
        float flowBlend = abs(flowPhases.x*2.0);
        output.varFlowData = float3(flowPhases * flowOffset, flowBlend);
    #endif

    float4x4 mwpWOtranslate = float4x4(worldViewProjMatrix[0], worldViewProjMatrix[1], worldViewProjMatrix[2], float4(0.0, 0.0, 0.0, 1.0));
    float4 vecPos = mul(float4(input.position.xyz, 1.0), mwpWOtranslate);
    output.position = float4(vecPos.x, vecPos.y, vecPos.w - 0.0001, vecPos.w);

    output.varTexCoord0.xy = input.texcoord0;

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif

    return output;
}
