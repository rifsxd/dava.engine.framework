#include "common.slh"
#include "blending.slh"
#include "lighting.slh"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float2 varTexCoord0 : TEXCOORD1;
    #endif

    #if MATERIAL_DECAL || (MATERIAL_LIGHTMAP  && VIEW_DIFFUSE) || ALPHA_MASK
        float2 varTexCoord1 : TEXCOORD2;
    #endif

    #if MATERIAL_DETAIL
        float2 varDetailTexCoord : TEXCOORD3;
    #endif

    #if TILED_DECAL_MASK
        float2 varDecalTileTexCoord : TEXCOORD3;
    #endif

    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        float2 varDetailNormalTexCoord : TEXCOORD4;
    #endif

    #if VERTEX_COLOR
        [lowp] half4 varVertexColor : COLOR1;
    #endif

    #if FLOWMAP
        [lowp] float3 varFlowData : TEXCOORD5; // For flowmap animations - xy next frame uv. z - frame time
    #endif
};

fragment_out
{
    float4 gbufferDepth : SV_TARGET0;
    float4 gbuffer0 : SV_TARGET1;
    float4 gbuffer1 : SV_TARGET2;
    float4 gbuffer2 : SV_TARGET3;
};


#if MATERIAL_TEXTURE || VERTEX_DISTORTION
    uniform sampler2D albedo;
#endif

#if MATERIAL_DECAL
    uniform sampler2D decal;
#endif

#if ALPHA_MASK
    uniform sampler2D alphamask;
#endif

#if MATERIAL_DETAIL
    uniform sampler2D detail;
#endif

#if MATERIAL_LIGHTMAP  && VIEW_DIFFUSE
    uniform sampler2D lightmap;
#endif

#if FLOWMAP
    uniform sampler2D flowmap;
#endif

#if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
    uniform sampler2D detailNormalMap;
    uniform sampler2D detailMask;
    [material][a] property float detailMaskValueOffset = 0.0;
    [material][a] property float detailNormalScale = 1.0;
#endif

#if MATERIAL_TEXTURE && ALPHATEST && ALPHATESTVALUE
    [material][a] property float alphatestThreshold = 0.0;
#endif

#if MATERIAL_TEXTURE && ALPHASTEPVALUE && ALPHABLEND
    [material][a] property float alphaStepValue = 0.5;
#endif

#if VERTEX_DISTORTION
    [auto][a] property float globalTime;
    [material][a] property float2 distortionTextureShiftSpeed;

    uniform sampler2D uvDistortionRG_distortionMaskB_Tex;
#endif

#if TILED_DECAL_MASK
    uniform sampler2D decalmask;
    uniform sampler2D decaltexture;
    [material][a] property float4 decalTileColor = float4(1.0, 1.0, 1.0, 1.0) ;
#endif

#if FLATCOLOR || FLATALBEDO
    [material][a] property float4 flatColor = float4(0, 0, 0, 0);
#endif

#if SETUP_LIGHTMAP && (MATERIAL_DECAL || MATERIAL_LIGHTMAP)
    [material][a] property float lightmapSize = 1.0;
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

    // FETCH PHASE
    #if MATERIAL_TEXTURE
        #if ALPHATEST || ALPHABLEND
            #if FLOWMAP
                float2 flowtc = input.varTexCoord0.xy;
                float3 flowData = input.varFlowData;
                float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;
                half4 flowSample1 = half4(tex2D(albedo, input.varTexCoord0.xy + flowDir*flowData.x));
                half4 flowSample2 = half4(tex2D(albedo, input.varTexCoord0.xy + flowDir*flowData.y));
                half4 textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z));
            #else
                float2 albedoUv = input.varTexCoord0.xy;

                #if VERTEX_DISTORTION
                    float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
                    half4 textureColor0 = half4(distortedColor);
                #else
                    half4 textureColor0 = half4(tex2D( albedo, albedoUv ));
                #endif
            #endif
            
            #if ALPHA_MASK 
                textureColor0.a *= FP_A8(tex2D(alphamask, input.varTexCoord1));
            #endif

          #else
            #if FLOWMAP
                float2 flowtc = input.varTexCoord0;
                float3 flowData = input.varFlowData;
                float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;
                half3 flowSample1 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.x).rgb);
                half3 flowSample2 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.y).rgb);
                half4 textureColor0;
                textureColor0.xyz = lerp(flowSample1, flowSample2, half(flowData.z));
                textureColor0.w = 1.0;
            #else
                #if TEST_OCCLUSION
                    half4 preColor = half4(tex2D( albedo, input.varTexCoord0 ) );
                    half4 textureColor0 = half4(preColor * preColor.a);
                #elif VERTEX_DISTORTION
                    float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
                    half4 textureColor0 = half4(distortedColor);
                #else
                    half4 textureColor0 = half4(tex2D(albedo, input.varTexCoord0));
                #endif
                    // originally, textureColor0 was half3, so we will just set .w component to 1.0
                    textureColor0.w = 1.0;
            #endif
        #endif
    #endif

    #if FLATALBEDO
        textureColor0 *= half4(flatColor);
    #endif

    #if MATERIAL_TEXTURE
        #if ALPHATEST
            float alpha = textureColor0.a;
            #if VERTEX_COLOR
                alpha *= float(input.varVertexColor.a);
            #endif
            #if ALPHATESTVALUE
                if( alpha < alphatestThreshold ) discard;
            #else
                if( alpha < 0.5 ) discard;
            #endif
        #endif

        #if ALPHASTEPVALUE && ALPHABLEND
            textureColor0.a = half(step(alphaStepValue, float(textureColor0.a)));
        #endif
    #endif

    #if MATERIAL_DECAL
        half3 textureColor1 = half3(tex2D( decal, input.varTexCoord1 ).rgb);
    #endif
    
    #if MATERIAL_LIGHTMAP  && VIEW_DIFFUSE
        half3 textureColor1 = half3(tex2D( lightmap, input.varTexCoord1 ).rgb);
    #endif
    
    #if MATERIAL_DETAIL
        half3 detailTextureColor = half3(tex2D( detail, input.varDetailTexCoord ).rgb);
    #endif

    #if MATERIAL_DECAL || MATERIAL_LIGHTMAP
        #if SETUP_LIGHTMAP
            half3 lightGray = float3(0.75, 0.75, 0.75);
            half3 darkGray = float3(0.25, 0.25, 0.25);

            bool isXodd;
            bool isYodd;

            if(frac(floor(input.varTexCoord1.x*lightmapSize)/2.0) == 0.0)
            {
                isXodd = true;
            }
            else
            {
                isXodd = false;
            }
            
            if(frac(floor(input.varTexCoord1.y*lightmapSize)/2.0) == 0.0)
            {
                isYodd = true;
            }
            else
            {
                isYodd = false;
            }

            if((isXodd && isYodd) || (!isXodd && !isYodd))
            {
                textureColor1 = lightGray;
            }
            else
            {
                textureColor1 = darkGray;
            }
        #endif
    #endif

    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        float detailMaskSample = FP_A8(tex2D(detailMask, input.varTexCoord0.xy));
        detailMaskSample = lerp(detailMaskValueOffset, 1.0f, detailMaskSample);
        float3 detailNSMapSample = tex2D(detailNormalMap, input.varDetailNormalTexCoord).xyz;
        float detailSpecularValue = lerp(0.5f, detailNSMapSample.z, detailMaskSample);
        float2 detailNormalValue = detailNSMapSample.xy * 2.0f - 1.0f;
        detailNormalValue *= detailMaskSample * detailNormalScale;
    #endif

    // DRAW PHASE

    #if MATERIAL_DECAL || MATERIAL_LIGHTMAP
        half3 color = half3(0.0, 0.0, 0.0);

        #if VIEW_ALBEDO
            color = half3(textureColor0.rgb);
        #else
            color = half3(1.0, 1.0, 1.0);
        #endif

        #if VIEW_DIFFUSE
            #if VIEW_ALBEDO
                color *= half3(textureColor1.rgb * 2.0);
            #else
                //do not scale lightmap in view diffuse only case. artist request
                color *= half3(textureColor1.rgb); 
            #endif
        #endif

    #elif MATERIAL_TEXTURE
        half3 color = half3(textureColor0.rgb);
    #else
        half3 color = half3(1.0, 1.0, 1.0);
    #endif
    
    #if TILED_DECAL_MASK
        half maskSample = FP_A8(tex2D(decalmask, input.varTexCoord0));
        half4 tileColor = half4(tex2D(decaltexture, input.varDecalTileTexCoord).rgba * decalTileColor);
        color.rgb += (tileColor.rgb - color.rgb) * tileColor.a * maskSample;
    #endif

    #if MATERIAL_DETAIL
        color *= detailTextureColor.rgb * 2.0;
    #endif

    #if VERTEX_COLOR
        color *= half3(input.varVertexColor.rgb);
    #endif

    #if FLATCOLOR
        color *= half3(flatColor.rgb);
    #endif

    output.gbuffer0 = float4(float3(color), 0.0);
    output.gbuffer1 = 0.0;
    output.gbuffer2 = 0.0;
    output.gbufferDepth.r = input.projectedPosition.z / input.projectedPosition.w * ndcToZMappingScale + ndcToZMappingOffset;
    output.gbufferDepth.gba = 0.0;

    return output;
}