#include "common.slh"
#include "blending.slh"

fragment_in
{
    [lowp] half4 varVertexColor : COLOR1;

    #if PARTICLES_PERSPECTIVE_MAPPING
        float3 varTexCoord0 : TEXCOORD0;
    #else
        float2 varTexCoord0 : TEXCOORD0;
    #endif

    #if FRAME_BLEND || PARTICLES_FLOWMAP_ANIMATION
        float2 varTexCoord1 : TEXCOORD1;
    #endif

    #if PARTICLES_FLOWMAP || PARTICLES_FLOWMAP_ANIMATION
        float2 varParticleFlowTexCoord : TEXCOORD2;
    #endif

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif

    #if PARTICLES_NOISE
        #if PARTICLES_FRESNEL_TO_ALPHA
            float4 varTexcoord6 : TEXCOORD6; // Noise uv and scale. Fresnel a.
        #else
            float3 varTexcoord6 : TEXCOORD6; // Noise uv and scale.
        #endif
    #elif PARTICLES_FRESNEL_TO_ALPHA
        float varTexcoord6 : TEXCOORD6; // Fresnel a.
    #endif

    #if FRAME_BLEND && PARTICLES_ALPHA_REMAP
        half2 varTexcoord3 : TEXCOORD3;
    #elif FRAME_BLEND || PARTICLES_ALPHA_REMAP || PARTICLES_FLOWMAP_ANIMATION
        half varTexcoord3 : TEXCOORD3;
    #endif
    
    #if PARTICLES_FLOWMAP || PARTICLES_FLOWMAP_ANIMATION
        float3 varFlowData : TEXCOORD4;
    #endif

    #if SOFT_PARTICLES
        float4 projectedPosition : TEXCOORD7;
    #endif
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D albedo;

#if PARTICLES_NOISE
    uniform sampler2D noiseTex;
#endif

#if PARTICLES_ALPHA_REMAP
    uniform sampler2D alphaRemapTex;
#endif

#if PARTICLES_FLOWMAP
    uniform sampler2D flowmap;
#endif

#if SOFT_PARTICLES
    uniform sampler2D dynamicTextureSrc0;
    [material][a] property float depthDifferenceSlope = 2.0;
#endif

#if ALPHASTEPVALUE && ALPHABLEND
    [material][a] property float alphaStepValue = 0.5;
#endif

#if PARTICLES_THREE_POINT_GRADIENT
    [material][a] property float4 gradientColorForWhite = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [material][a] property float4 gradientColorForBlack = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [material][a] property float4 gradientColorForMiddle = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [material][a] property float gradientMiddlePoint = 0.5f;
#endif

#if FLATCOLOR || FLATALBEDO
    [material][a] property float4 flatColor = float4(0, 0, 0, 0);
#endif

#if PARTICLE_DEBUG_SHOW_ALPHA
    [material][a] property float particleAlphaThreshold = 0.2f;
    [material][a] property float4 particleDebugShowAlphaColor =  float4(0.0f, 0.0f, 1.0f, 0.4f);
#endif

#if SOFT_PARTICLES
    [auto][a] property float4x4 invProjMatrix;
    [auto][a] property float4x4 invViewProjMatrix;
    [auto][a] property float2 viewportSize;
    [auto][a] property float2 viewportOffset;
    [auto][a] property float2 renderTargetSize;
#endif

#if ALPHABLEND && ALPHA_EROSION
    [material][a] property float alphaErosionAcceleration = 2.0f;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    #if VERTEX_FOG
        float varFogAmoung = float(input.varFog.a);
        float3 varFogColor = float3(input.varFog.rgb);
    #endif
    
    // FETCH PHASE
    half4 textureColor0 = half4(1.0, 0.0, 0.0, 1.0);
    #if ALPHATEST || ALPHABLEND
        #if PARTICLES_FLOWMAP && !PARTICLES_FLOWMAP_ANIMATION
            float2 flowtc = input.varParticleFlowTexCoord;
            float3 flowData = input.varFlowData;
            float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;

            #if PARTICLES_NOISE
                flowDir *= tex2D(noiseTex, input.varTexcoord6.xy).r * input.varTexcoord6.z;
            #endif

            half4 flowSample1 = half4(tex2D(albedo, input.varTexCoord0.xy + flowDir*flowData.x));
            half4 flowSample2 = half4(tex2D(albedo, input.varTexCoord0.xy + flowDir*flowData.y));
            textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z));
        #elif (PARTICLES_FLOWMAP && PARTICLES_FLOWMAP_ANIMATION)
            float2 offsetVectorCurr = tex2D(flowmap, input.varParticleFlowTexCoord.xy).xy;
            offsetVectorCurr = offsetVectorCurr * 2.0f - 1.0f;
            offsetVectorCurr *= input.varTexcoord3 * input.varFlowData.z; // input.varTexcoord3 - frame time normalized. varFlowData - distortion power.

            float2 offsetVectorNext = tex2D(flowmap, input.varFlowData.xy).xy;
            offsetVectorNext = offsetVectorNext * 2.0f - 1.0f;
            offsetVectorNext *= (1 - input.varTexcoord3) * input.varFlowData.z;

            half4 albedoSample = tex2D(albedo, input.varTexCoord0.xy - offsetVectorCurr);
            half4 albedoSampleNext = tex2D(albedo, input.varTexCoord1.xy + offsetVectorNext);

            textureColor0 = lerp(albedoSample, albedoSampleNext, input.varTexcoord3);
        #else
            float2 albedoUv = input.varTexCoord0.xy;
            #if PARTICLES_NOISE
                #if PARTICLES_PERSPECTIVE_MAPPING
                    float noiseSample = tex2D(noiseTex, input.varTexcoord6.xy / input.varTexCoord0.z).r * 2.0f - 1.0f;
                    noiseSample *= input.varTexCoord0.z;
                #else
                    float noiseSample = tex2D(noiseTex, input.varTexcoord6.xy).r * 2.0f - 1.0f;
                #endif
                noiseSample *= input.varTexcoord6.z;
                albedoUv.xy += float2(noiseSample, noiseSample);
            #endif
            #if PARTICLES_PERSPECTIVE_MAPPING
                textureColor0 = half4(tex2D(albedo, albedoUv / input.varTexCoord0.z));
            #elif VERTEX_DISTORTION
                float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
                textureColor0 = half4(distortedColor);
            #else
                textureColor0 = half4(tex2D(albedo, albedoUv));
            #endif
        #endif

        #if PARTICLES_ALPHA_REMAP
            #if FRAME_BLEND
                float4 remap = tex2D(alphaRemapTex, float2(half(textureColor0.a), input.varTexcoord3.y));
            #else
                float4 remap = tex2D(alphaRemapTex, float2(half(textureColor0.a), input.varTexcoord3));
            #endif
            textureColor0.a = remap.r;
        #endif
    #else
        #if PARTICLES_FLOWMAP
            float2 flowtc = input.varParticleFlowTexCoord;
            float3 flowData = input.varFlowData;
            float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;
            half3 flowSample1 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.x).rgb);
            half3 flowSample2 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.y).rgb);
            textureColor0.rgb = lerp(flowSample1, flowSample2, half(flowData.z));
        #else
            #if TEST_OCCLUSION
                half4 preColor = half4(tex2D(albedo, input.varTexCoord0));
                textureColor0.rgb = half3(preColor.rgb*preColor.a);
            #else
                textureColor0.rgb = half3(tex2D(albedo, input.varTexCoord0).rgb);
            #endif
        #endif
    #endif
    
    #if FRAME_BLEND
        #if PARTICLES_PERSPECTIVE_MAPPING
            half4 blendFrameColor = half4(tex2D(albedo, input.varTexCoord1 / input.varTexCoord0.z));
        #else
            half4 blendFrameColor = half4(tex2D(albedo, input.varTexCoord1));
        #endif
        #if PARTICLES_ALPHA_REMAP
            half varTime = input.varTexcoord3.x;
        #else
            half varTime = input.varTexcoord3;
        #endif
        textureColor0 = lerp(textureColor0, blendFrameColor, varTime);
    #endif

    #if PARTICLES_THREE_POINT_GRADIENT
        half uperGradientLerpValue = textureColor0.r - gradientMiddlePoint;
        float gradientMiddlePointValue = clamp(gradientMiddlePoint, 0.001f, 0.999f);
        half4 lowerGradColor = lerp(gradientColorForBlack, gradientColorForMiddle, textureColor0.r / gradientMiddlePointValue);
        half4 upperGradColor = lerp(gradientColorForMiddle, gradientColorForWhite, uperGradientLerpValue / (1.0f - gradientMiddlePointValue));
        half4 finalGradientColor = lerp(lowerGradColor, upperGradColor, step(0.0f, uperGradientLerpValue));
        textureColor0 = half4(finalGradientColor.rgb, textureColor0.a * finalGradientColor.a);
    #endif
    
    #if FLATALBEDO
        textureColor0 *= half4(flatColor);
    #endif
    
    #if ALPHATEST
        float alpha = textureColor0.a;
        alpha *= float(input.varVertexColor.a);

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
    
    output.color = float4(textureColor0);

    #if !ALPHABLEND
        output.color.w = 1.0;
    #endif

    output.color *= float4(input.varVertexColor);

    #if FLATCOLOR
        output.color *= flatColor;
    #endif

    #if ALPHABLEND && ALPHA_EROSION
        float srcA = tex2D(albedo, albedoUv).a;
        float opacity = saturate(1. - input.varVertexColor.a);
        output.color.a = (srcA - (alphaErosionAcceleration + 1. - srcA * alphaErosionAcceleration) * opacity);
    #endif

    #if PARTICLES_FRESNEL_TO_ALPHA
        #if PARTICLES_NOISE
            output.color.a *= input.varTexcoord6.w;
        #else
            output.color.a *= input.varTexcoord6;
        #endif
    #endif

    #if VERTEX_FOG
        output.color.rgb = lerp(output.color.rgb, varFogColor, varFogAmoung);
    #endif

    #if PARTICLE_DEBUG_SHOW_ALPHA
        if (output.color.a < particleAlphaThreshold)
            output.color = particleDebugShowAlphaColor;
        else
            output.color = 0.0;
    #endif

    #if SOFT_PARTICLES
        float4 projectedPosition = input.projectedPosition / input.projectedPosition.w;
        #if DEPTH_TARGET_IS_FRAMEBUFFER
            float2 depthSampleUv = projectedPosition.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
        #else
            float2 depthSampleUv = projectedPosition.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
        #endif
        
        depthSampleUv = (depthSampleUv * viewportSize + centerPixelMapping + viewportOffset) / renderTargetSize;
        float depthSample = (tex2D(dynamicTextureSrc0, depthSampleUv).x - ndcToZMappingOffset) / ndcToZMappingScale;
        float4 sampledPosition = mul(float4(projectedPosition.xy, depthSample, 1.0), invProjMatrix);
        float4 currentPosition = mul(projectedPosition, invProjMatrix);
        float curDepth = currentPosition.z / max(currentPosition.w, 0.0001);
        float sampledDepth = sampledPosition.z / max(sampledPosition.w, 0.0001);
        float distanceDifference = max(0.0, curDepth  - sampledDepth);
        float scale = 1.0 - exp(-depthDifferenceSlope * distanceDifference * distanceDifference);
        #if (BLENDING == BLENDING_ADDITIVE)
            output.color *= scale;
        #else
            output.color.w *= scale;
        #endif
    #endif

    #if PARTICLE_DEBUG_SHOW_OVERDRAW
        output.color = float4(0.01f, 0.0f, 0.0f, 1.0f);
    #endif

    return output;
}
