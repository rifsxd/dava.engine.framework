#include "common.slh"
#include "blending.slh"
#if RECEIVE_SHADOW
    #include "shadow-mapping.slh"
#endif

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
    [lowp] half4 varVertexColor : COLOR1;

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif
    
    #if RECEIVE_SHADOW
        float4 worldPos : COLOR2;
        float4 projPos : COLOR3;
    #endif
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D albedo;

#if ALPHATEST && ALPHATESTVALUE
    [material][a] property float alphatestThreshold = 0.0;
#endif

#if ALPHASTEPVALUE && ALPHABLEND
    [material][a] property float alphaStepValue = 0.5;
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

    #if VERTEX_FOG
        float varFogAmoung = float(input.varFog.a);
        float3 varFogColor = float3(input.varFog.rgb);
    #endif

    half4 textureColor0 = half4(1.0, 0.0, 0.0, 1.0);
    #if ALPHATEST || ALPHABLEND
        #if VERTEX_DISTORTION
            float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
            textureColor0 = half4(distortedColor);
        #else
            textureColor0 = half4(tex2D(albedo, input.varTexCoord0.xy));
        #endif
    #else
        #if TEST_OCCLUSION
            half4 preColor = half4(tex2D(albedo, input.varTexCoord0));
            textureColor0.rgb = half3(preColor.rgb*preColor.a);
        #elif VERTEX_DISTORTION
            float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
            textureColor0.rgb = half3(distortedColor.rgb);
        #else
            textureColor0.rgb = half3(tex2D(albedo, input.varTexCoord0).rgb);
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
    
    #if ALPHASTEPVALUE && ALPHABLEND
        textureColor0.a = half(step(alphaStepValue, float(textureColor0.a)));
    #endif

    // DRAW PHASE
    #if ALPHABLEND
        output.color = float4(textureColor0);
    #else
        output.color = float4(float3(textureColor0.rgb), 1.0);
    #endif

    output.color *= float4(input.varVertexColor);

    #if FLATCOLOR
        output.color *= flatColor;
    #endif
    
    #if RECEIVE_SHADOW
        float4 shadowMapInfo;
        shadowMapInfo = getCascadedShadow(input.worldPos, input.projPos, float3(0.0, 1.0, 0.0), 0.5);
        float3 shadowMapColor;
        shadowMapColor = getShadowColor(shadowMapInfo);
        output.color.rgb *= shadowMapColor;
    #endif

    #if VERTEX_FOG
        output.color.rgb = lerp(output.color.rgb, varFogColor, varFogAmoung);
    #endif

    return output;
}
