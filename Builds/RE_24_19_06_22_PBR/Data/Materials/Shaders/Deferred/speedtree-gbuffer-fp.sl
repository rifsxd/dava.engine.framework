#include "common.slh"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;
    float2 varTexCoord0 : TEXCOORD1;
    [lowp] half4 varVertexColor : COLOR1;
};

fragment_out
{
    float4 gbufferDepth : SV_TARGET0;
    float4 gbuffer0 : SV_TARGET1;
    float4 gbuffer1 : SV_TARGET2;
    float4 gbuffer2 : SV_TARGET3;
};

uniform sampler2D albedo;

#if ALPHATEST && ALPHATESTVALUE
    [material][a] property float alphatestThreshold = 0.0;
#endif

#if VERTEX_DISTORTION
    [auto][a] property float globalTime;
    [material][a] property float2 distortionTextureShiftSpeed;

    uniform sampler2D uvDistortionRG_distortionMaskB_Tex;
#endif

#if FLATCOLOR || FLATALBEDO
    [material][a] property float4 flatColor = float4(0, 0, 0, 0);
#endif

#if VERTEX_DISTORTION
    inline float4 CalculateDistortion(float2 uv, float time, float2 distortionShiftSpeed)
    {
        // Use r, g as displacement. Uv are moved with different speed. Use b channel as mask for displacement.
        float distortionR = tex2D(uvDistortionRG_distortionMaskB_Tex, uv + float2(0.0f, time * distortionShiftSpeed.x)).r * 2.0 - 1.0;
        float distortionG = tex2D(uvDistortionRG_distortionMaskB_Tex, uv + float2(0.0f, time * distortionShiftSpeed.y)).g * 2.0 - 1.0;
        float distortionMask = tex2D(uvDistortionRG_distortionMaskB_Tex, uv).b;
        uv.y += (distortionR + distortionG) * distortionMask;
        return tex2D(albedo, uv);
    }
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    #if ALPHATEST
        #if VERTEX_DISTORTION
            float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
            half4 textureColor0 = half4(distortedColor);
        #else
            half4 textureColor0 = half4(tex2D(albedo, input.varTexCoord0.xy));
        #endif
    #else
        #if TEST_OCCLUSION
            half4 preColor = half4(tex2D(albedo, input.varTexCoord0));
            half3 textureColor0 = half3(preColor.rgb * preColor.a);
        #elif VERTEX_DISTORTION
            float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
            half3 textureColor0 = half3(distortedColor.rgb);
        #else
            half3 textureColor0 = half3(tex2D(albedo, input.varTexCoord0).rgb);
        #endif
    #endif

    #if FLATALBEDO
        textureColor0 *= half4(flatColor);
    #endif

    #if ALPHATEST
        float alpha = textureColor0.a * float(input.varVertexColor.a);

        #if ALPHATESTVALUE
            if(alpha < alphatestThreshold) discard;
        #else
            if(alpha < 0.5) discard;
        #endif
    #endif

    // DRAW PHASE

    float3 color = float3(textureColor0.rgb);

    color *= float3(input.varVertexColor.xyz);

    #if FLATCOLOR
        color *= flatColor.xyz;
    #endif

    output.gbuffer0 = float4(color, 0.0);
    output.gbuffer1 = 0.0;
    output.gbuffer2 = 0.0;
    output.gbufferDepth.r = input.projectedPosition.z / input.projectedPosition.w * ndcToZMappingScale + ndcToZMappingOffset;
    output.gbufferDepth.gba = 0.0;

    return output;
}
