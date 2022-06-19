#include "common.slh"
#include "blending.slh"
#include "lighting.slh"
#if RECEIVE_SHADOW
    #include "shadow-mapping.slh"
#endif

fragment_in
{
    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float4 varTexCoord0 : TEXCOORD0;
    #endif

    #if MATERIAL_DECAL || ALPHA_MASK
        float2 varTexCoord1 : TEXCOORD1;
    #endif

    #if MATERIAL_DETAIL
        float2 varDetailTexCoord : TEXCOORD2;
    #endif

    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        float2 varDetailNormalTexCoord : TEXCOORD3;
    #endif

    #if VERTEX_LIT
        [lowp] half3 varDiffuseColor : COLOR0;

        #if BLINN_PHONG
            [lowp] half varSpecularColor : TEXCOORD4;
        #elif NORMALIZED_BLINN_PHONG
            [lowp] half4 varSpecularColor : TEXCOORD4;
        #endif

    #elif PIXEL_LIT

        #if FAST_NORMALIZATION
            [lowp] half3 varHalfVec : COLOR0;
        #endif

        [lowp] half3 varToLightVec : COLOR1;
        float3 varToCameraVec : TEXCOORD7;
        float4 tangentToView0 : NORMAL1;
        float4 tangentToView1 : NORMAL2;
        float4 tangentToView2 : NORMAL3;
        
        #if RECEIVE_SHADOW
            float3 tangentToWorld0 : TANGENTTOWORLD0;
            float3 tangentToWorld1 : TANGENTTOWORLD1;
            float3 tangentToWorld2 : TANGENTTOWORLD2;
        #endif
        
    #endif

    #if VERTEX_COLOR
        [lowp] half4 varVertexColor : COLOR1;
    #endif

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif

    #if GEO_DECAL
        float2 geoDecalCoord : TEXCOORD6;
    #endif

    #if (ENVIRONMENT_MAPPING && (!ENVIRONMENT_MAPPING_NORMALMAP))
        float3 reflectionVector : TEXCOORD7;
    #endif
    
    #if RECEIVE_SHADOW
        float4 projPos : COLOR2;
        #if VERTEX_LIT
            float4 worldNormalNdotL : COLOR3;
        #endif
        float4 worldPos : POSITION3;
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

#if ENVIRONMENT_MAPPING
    uniform samplerCUBE cubemap;
    [material][a] property float3 cubemapIntensity = float3(1.0, 1.0, 1.0);
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

#if PIXEL_LIT
    uniform sampler2D normalmap;
    #if GEO_DECAL_SPECULAR
        uniform sampler2D specularmap;
    #endif
    [material][a] property float inSpecularity = 1.0;
    [material][a] property float3 metalFresnelReflectance = float3(0.5, 0.5, 0.5);
    [material][a] property float normalScale = 1.0;

    [auto][a] property float4 lightPosition0;
    [auto][a] property float4x4 pointLights; // 0,1:(position, radius); 2,3:(color, unused)
#endif

#if VERTEX_DISTORTION
    [auto][a] property float globalTime;
    [material][a] property float2 distortionTextureShiftSpeed;

    uniform sampler2D uvDistortionRG_distortionMaskB_Tex;
#endif

#if TILED_DECAL_MASK
    uniform sampler2D decalmask;
    uniform sampler2D decaltexture;
    [material][a] property float4 decalTileColor = float4(1.0, 1.0, 1.0, 1.0);
#endif

    [auto][a] property float3 lightAmbientColor0;
    [auto][a] property float3 lightColor0;
[auto][a] property float4x4 invViewMatrix;

    #if NORMALIZED_BLINN_PHONG && VIEW_SPECULAR
        [material][a] property float inGlossiness = 0.5;
    #endif

#if FLATCOLOR || FLATALBEDO
    [material][a] property float4 flatColor = float4(0, 0, 0, 0);
#endif

#if SETUP_LIGHTMAP && MATERIAL_DECAL
    [material][a] property float lightmapSize = 1.0;
#endif

inline float FresnelShlick(float NdotL, float Cspec)
{
    float expf = 5.0;
    return Cspec + (1.0 - Cspec) * pow(1.0 - NdotL, expf);
}

inline float3 FresnelShlickVec3(float NdotL, float3 Cspec)
{
    float expf = 5.0;
    return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, expf));
}

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
        float3 varFogColor  = float3(input.varFog.rgb);
    #endif
    
    // FETCH PHASE
    half4 textureColor0 = half4(1.0, 0.0, 0.0, 1.0);
    #if GEO_DECAL
        textureColor0 = half4(tex2D(albedo, input.geoDecalCoord));
    #elif MATERIAL_TEXTURE
        #if PIXEL_LIT || ALPHATEST || ALPHABLEND || VERTEX_LIT
            float2 albedoUv = input.varTexCoord0.xy;

            #if VERTEX_DISTORTION
                float4 distortedColor = CalculateDistortion(input.varTexCoord0.xy, globalTime, distortionTextureShiftSpeed);
                textureColor0 = half4(distortedColor);
            #else
                textureColor0 = half4(tex2D(albedo, albedoUv));
            #endif

            #if ALPHA_MASK 
                textureColor0.a *= FP_A8(tex2D(alphamask, input.varTexCoord1));
            #endif
        #else
            #if TEST_OCCLUSION
                half4 preColor = half4(tex2D(albedo, input.varTexCoord0.xy));
                textureColor0.rgb = half3(preColor.rgb*preColor.a);
            #elif VERTEX_DISTORTION
                float4 distortedColor = CalculateDistortion(input.varTexCoord0.xy, globalTime, distortionTextureShiftSpeed);
                textureColor0.rgb = half3(distortedColor.rgb);
            #else
                textureColor0.rgb = half3(tex2D(albedo, input.varTexCoord0.xy).rgb);
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

    #if MATERIAL_DETAIL
        half3 detailTextureColor = half3(tex2D(detail, input.varDetailTexCoord).rgb);
    #endif

    #if MATERIAL_DECAL
        half3 textureColor1 = half3(tex2D(decal, input.varTexCoord1).rgb);

        #if SETUP_LIGHTMAP
            half3 lightGray = half3(0.75, 0.75, 0.75);
            half3 darkGray = half3(0.25, 0.25, 0.25);
    
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

        #if GEO_DECAL_SPECULAR
            float specularSample = FP_A8(tex2D(specularmap, input.geoDecalCoord));
        #else
            float specularSample = textureColor0.a;
            #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
                specularSample *= detailSpecularValue * 2.0;
            #endif
        #endif

    half3 color = half3(0.0, 0.0, 0.0);
    #if VERTEX_LIT
        #if BLINN_PHONG || NORMALIZED_BLINN_PHONG
            #if VIEW_AMBIENT
                color += half3(lightAmbientColor0);
            #endif

            #if VIEW_DIFFUSE
                color += input.varDiffuseColor;
            #endif

            #if VIEW_ALBEDO
                #if TILED_DECAL_MASK
                    half maskSample = FP_A8(tex2D(decalmask, input.varTexCoord0.xy));
                    half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
                    color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
                #else
                    color *= textureColor0.rgb;
                #endif
            #endif
        #endif

        #if VIEW_SPECULAR
            #if BLINN_PHONG
                color += half3((input.varSpecularColor * specularSample) * lightColor0);
            #elif NORMALIZED_BLINN_PHONG
                float glossiness = pow(5000.0, inGlossiness * specularSample);
                float specularNorm = (glossiness + 2.0) / 8.0;
                float3 spec = float3(input.varSpecularColor.xyz * pow(float(input.varSpecularColor.w), glossiness) * specularNorm);
                color += half3(spec * lightColor0);
            #endif
        #endif

    #elif PIXEL_LIT
        // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
        float3 normal = tex2D(normalmap, input.varTexCoord0.xy).rgb * 2.0 - 1.0;
        #if RECEIVE_SHADOW
            float3 normalShady = normal;
        #endif
        
        normal.xy *= normalScale;
        normal = normalize(normal);
        #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
            normal = normalize(float3(normal.xy + detailNormalValue.xy, normal.z));
        #endif
    
        float3 viewNormal = float3(dot(input.tangentToView0.xyz, normal), dot(input.tangentToView1.xyz, normal), dot(input.tangentToView2.xyz, normal));
        float3 toCameraNormalized = float3(normalize(input.varToCameraVec));
    
        float NdotL = 0.0f;
        #if FAST_NORMALIZATION
            // Kwasi normalization :-)
            // compute diffuse lighting
            float3 normalizedHalf = float3(normalize(input.varHalfVec.xyz));
            
            NdotL = max(dot(normal, float3(input.varToLightVec.xyz)), 0.0);
            float NdotH = max(dot(normal, normalizedHalf), 0.0);
            float LdotH = max(dot(float3(input.varToLightVec.xyz), normalizedHalf), 0.0);
            float NdotV = max(dot(normal, input.varToCameraVec), 0.0);
        #else
            float3 toLightNormalized = float3(normalize(input.varToLightVec.xyz));
            float3 H = toCameraNormalized + toLightNormalized;
            H = normalize(H);

            // compute diffuse lighting
            NdotL = max(dot(normal, toLightNormalized), 0.0);
            float NdotH = max(dot(normal, H), 0.0);
            float LdotH = max(dot(toLightNormalized, H), 0.0);
            float NdotV = max(dot(normal, toCameraNormalized), 0.0);
        #endif
    
        float3 diffuse = lightColor0 * (NdotL / _PI);
        float3 specular = 0.0;

        #if NORMALIZED_BLINN_PHONG
            #if DIELECTRIC
                float fresnelOut = FresnelShlick(NdotV, dielectricFresnelReflectance);
                fresnelOut = FresnelShlick(NdotV, dielectricFresnelReflectance);
            #else
                #if FAST_METAL
                    float fresnelOut = FresnelShlick(NdotV, (metalFresnelReflectance.r + metalFresnelReflectance.g + metalFresnelReflectance.b) / 3.0);
                #else
                    float3 fresnelOut = FresnelShlickVec3(NdotV, metalFresnelReflectance);
                #endif
            #endif

            #if (MAX_POINT_LIGHTS > 0)
            {
                float3 viewPosition = float3(input.tangentToView0.w, input.tangentToView1.w, input.tangentToView2.w);
                diffuse += ApplyLight(pointLights[0], pointLights[2], viewPosition, viewNormal);
                #if (MAX_POINT_LIGHTS > 1)
                    diffuse += ApplyLight(pointLights[1], pointLights[3], viewPosition, viewNormal);
                #endif
            }
            #endif

            #if VIEW_SPECULAR
                float glossiness = inGlossiness * specularSample;
                float glossPower = pow(5000.0, glossiness);

                float specCutoff = NdotL;
                float specularNorm = (glossPower + 2.0) / 8.0;
                specular = lightColor0 * (specularNorm * pow(NdotH, glossPower) * specCutoff * inSpecularity);

                #if ENVIRONMENT_MAPPING
                    #if (ENVIRONMENT_MAPPING_NORMALMAP)
                        float3 reflected = -reflect(toCameraNormalized, normal);
                        float3 viewReflected = float3(
                            dot(reflected, input.tangentToView0.xyz), 
                            dot(reflected, input.tangentToView1.xyz), 
                            dot(reflected, input.tangentToView2.xyz));
                        float3 samplingDirection = mul(float4(viewReflected, 0.0), invViewMatrix).xyz;
                    #else
                        float3 samplingDirection = input.reflectionVector;
                    #endif
                    specular += texCUBE(cubemap, samplingDirection).xyz * cubemapIntensity * specularSample;
                #endif

                specular *= fresnelOut;
            #endif
        #endif
        
        #if RECEIVE_SHADOW
            float4 shadowInfo;
            float3 shadowColor;
                    
            normalShady.xy *= shadowLitNormalScale;
            normalShady = normalize(normalShady);
            float NdotLShady = 0.0f;
            #if FAST_NORMALIZATION
                NdotLShady = max(dot(normalShady, float3(input.varToLightVec.xyz)), 0.0);
            #else
                NdotLShady = max(dot(normalShady, toLightNormalized), 0.0);
            #endif
            float3 worldNormal = float3(dot(input.tangentToWorld0.xyz, normalShady), dot(input.tangentToWorld1.xyz, normalShady), dot(input.tangentToWorld2.xyz, normalShady));
            shadowInfo = getCascadedShadow(input.worldPos, input.projPos, worldNormal, NdotLShady);
            shadowColor = getShadowColor(shadowInfo);
            
            #if VIEW_DIFFUSE
                diffuse *= shadowLitDiffuseSpecAmbientMult.x + (1.0 - shadowLitDiffuseSpecAmbientMult.x) * shadowInfo.x;
            #endif
            #if VIEW_SPECULAR
                specular *= shadowLitDiffuseSpecAmbientMult.y + (1.0 - shadowLitDiffuseSpecAmbientMult.y) * shadowInfo.x;
            #endif
        #endif

        #if VIEW_AMBIENT
        #if RECEIVE_SHADOW
            color += half3(lightAmbientColor0 * saturate(shadowColor * shadowLitDiffuseSpecAmbientMult.z));
        #else
            color += half3(lightAmbientColor0);
        #endif
        #endif
    
        #if VIEW_DIFFUSE
            color += half3(diffuse);
        #endif

        #if VIEW_ALBEDO
            #if TILED_DECAL_MASK
                half maskSample = FP_A8(tex2D(decalmask, input.varTexCoord0.xy));

               #if TILED_DECAL_TRANSFORM
                    #if HARD_SKINNING
                        half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
                     #else
                        half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
                     #endif
               #else
                     half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
               #endif

                color *= textureColor0.rgb + (tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample;
            #else
                color *= textureColor0.rgb;
            #endif
        #endif

        #if VIEW_SPECULAR
            color += half3(specular);
        #endif
    #else
        #if MATERIAL_DECAL
            #if VIEW_ALBEDO
                color = half3(textureColor0.rgb);
            #else
                color = half3(1.0, 1.0, 1.0);
            #endif

            #if VIEW_DIFFUSE
                #if VIEW_ALBEDO
                    color *= half3(textureColor1.rgb * 2.0);
                #else
                    // do not scale lightmap in view diffuse only case. artist request
                    color *= half3(textureColor1.rgb); 
                #endif
            #endif
        #elif MATERIAL_TEXTURE
            color = half3(textureColor0.rgb);
        #else
            color = half3(1.0, 1.0, 1.0);
        #endif

        #if TILED_DECAL_MASK
            half maskSample = FP_A8(tex2D(decalmask, input.varTexCoord0.xy));

        #if TILED_DECAL_TRANSFORM 
            #if HARD_SKINNING
                half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
            #else
                half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
            #endif
        #else
            half4 tileColor = half4(tex2D(decaltexture, input.varTexCoord0.zw).rgba * decalTileColor);
        #endif

            color.rgb += (tileColor.rgb - color.rgb) * tileColor.a * maskSample;
        #endif
    #endif

    #if MATERIAL_DETAIL
        color *= detailTextureColor.rgb * 2.0;
    #endif

    #if ALPHABLEND && MATERIAL_TEXTURE
        output.color = float4(float3(color.rgb), textureColor0.a);
    #else
        output.color = float4(color.r, color.g, color.b, 1.0);
    #endif

    #if VERTEX_COLOR
        output.color *= float4(input.varVertexColor);
    #endif
        
    #if FLATCOLOR
        output.color *= flatColor;
    #endif
    
    #if RECEIVE_SHADOW && VERTEX_LIT
        float4 shadowInfo;
        float3 shadowColor;    
        shadowInfo = getCascadedShadow(input.worldPos, input.projPos, input.worldNormalNdotL.xyz, input.worldNormalNdotL.w);
        shadowColor = getShadowColor(shadowInfo);

        output.color.rgb *= shadowColor;
    #endif

    #if VERTEX_FOG
        output.color.rgb = lerp(output.color.rgb, varFogColor, varFogAmoung);
    #endif

    #if GEO_DECAL_DEBUG
        output.color += float4(0.75f, 0.75f, 0.75f, 1.0f);
    #endif
    return output;
}
