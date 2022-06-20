#include "common.slh"
#include "blending.slh"
#if RECEIVE_SHADOW
    #include "shadow-mapping.slh"
#endif

fragment_in
{
    [lowp] half4 varVertexColor : COLOR1;

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float2 varTexCoord0 : TEXCOORD0;
    #endif

    #if MATERIAL_DECAL || ALPHA_MASK
        float2 varTexCoord1 : TEXCOORD1;
    #endif

    #if MATERIAL_DETAIL
        float2 varDetailTexCoord : TEXCOORD2;
    #endif

    #if TILED_DECAL_MASK
        float2 varDecalTileTexCoord : TEXCOORD2;
    #endif

    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        float2 varDetailNormalTexCoord : TEXCOORD3;
    #endif

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif

    #if FLOWMAP
        float3 varFlowData : TEXCOORD4;
    #endif
    
    #if GEO_DECAL
        float2 geoDecalCoord : TEXCOORD6;
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
    
    // FETCH PHASE
    half4 textureColor0 = half4(1.0, 0.0, 0.0, 1.0);
    #if GEO_DECAL
        textureColor0 = half4(tex2D(albedo, input.geoDecalCoord));
    #elif MATERIAL_TEXTURE
        #if ALPHATEST || ALPHABLEND
            #if FLOWMAP
                float2 flowtc = input.varTexCoord0.xy;
                float3 flowData = input.varFlowData;
                float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;

                half4 flowSample1 = half4(tex2D(albedo, input.varTexCoord0.xy + flowDir*flowData.x));
                half4 flowSample2 = half4(tex2D(albedo, input.varTexCoord0.xy + flowDir*flowData.y));
                textureColor0 = lerp(flowSample1, flowSample2, half(flowData.z));
            #else
                float2 albedoUv = input.varTexCoord0.xy;
                
                #if VERTEX_DISTORTION
                    float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
                    textureColor0 = half4(distortedColor);
                #else
                    textureColor0 = half4(tex2D(albedo, albedoUv));
                #endif
            #endif
            
            #if ALPHA_MASK 
                textureColor0.a *= FP_A8(tex2D(alphamask, input.varTexCoord1));
            #endif
          #else // end of PIXEL_LIT
            #if FLOWMAP
                float2 flowtc = input.varTexCoord0;
                float3 flowData = input.varFlowData;
                float2 flowDir = float2(tex2D(flowmap, flowtc).xy) * 2.0 - 1.0;
                half3 flowSample1 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.x).rgb);
                half3 flowSample2 = half3(tex2D(albedo, input.varTexCoord0 + flowDir*flowData.y).rgb);
                textureColor0.rgb = lerp(flowSample1, flowSample2, half(flowData.z));
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
                if(alpha < alphatestThreshold) discard;
            #else
                if(alpha < 0.5) discard;
            #endif
        #endif
        
        #if ALPHASTEPVALUE && ALPHABLEND
            textureColor0.a = half(step(alphaStepValue, float(textureColor0.a)));
        #endif
    #endif

    #if MATERIAL_DECAL
        half3 textureColor1 = half3(tex2D(decal, input.varTexCoord1).rgb);
    #endif

    #if MATERIAL_DETAIL
        half3 detailTextureColor = half3(tex2D(detail, input.varDetailTexCoord).rgb);
    #endif

    // DRAW PHASE

    #if MATERIAL_DECAL
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

            #if TILED_DECAL_TRANSFORM
                #if HARD_SKINNING
                    half4 tileColor = half4(tex2D(decaltexture, input.varDecalTileTexCoord).rgba * decalTileColor);
                #else
                    half4 tileColor = half4(tex2D(decaltexture, input.varDecalTileTexCoord).rgba * decalTileColor);
                #endif
            #else
                half4 tileColor = half4(tex2D(decaltexture, input.varDecalTileTexCoord).rgba * decalTileColor);
            #endif

        color.rgb += (tileColor.rgb - color.rgb) * tileColor.a * maskSample;
    #endif

    #if MATERIAL_DETAIL
        color *= detailTextureColor.rgb * 2.0;
    #endif

    #if ALPHABLEND && MATERIAL_TEXTURE
        output.color = float4(float3(color.rgb), textureColor0.a);
    #else
        output.color = float4(float3(color.rgb), 1.0);
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

    #if GEO_DECAL_DEBUG
        output.color += float4(0.75f, 0.75f, 0.75f, 1.0f);
    #endif

    return output;
}
