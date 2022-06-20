#include "common.slh"
#include "blending.slh"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float2 varTexCoord0 : TEXCOORD1;
    #endif

    #if MATERIAL_DECAL || ALPHA_MASK
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

    #if PIXEL_LIT
        float3 tangentToView0 : NORMAL1; // here we are storing transposed (T, B, N) matrix
        float3 tangentToView1 : NORMAL2; // used for transforming normal from tangent space to view space
        float3 tangentToView2 : NORMAL3; // in .w components position in view-space is stored
    #else
        float3 normal : NORMAL0;
    #endif

    #if VERTEX_COLOR
        [lowp] half4 varVertexColor : COLOR1;
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

#if MATERIAL_DETAIL
    uniform sampler2D detail;
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

uniform sampler2D normalmap;
[material][a] property float inSpecularity = 1.0;
[material][a] property float3 metalFresnelReflectance = float3(0.5, 0.5, 0.5);
[material][a] property float normalScale = 1.0;

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

#if NORMALIZED_BLINN_PHONG && VIEW_SPECULAR
    [material][a] property float inGlossiness = 0.5;
#endif

#if FLATCOLOR || FLATALBEDO
    [material][a] property float4 flatColor = float4(0, 0, 0, 0);
#endif

#if SETUP_LIGHTMAP && MATERIAL_DECAL
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
        float2 albedoUv = input.varTexCoord0.xy;

        #if VERTEX_DISTORTION
            float4 distortedColor = CalculateDistortion(input.varTexCoord0, globalTime, distortionTextureShiftSpeed);
            half4 textureColor0 = half4(distortedColor);
        #else
            half4 textureColor0 = half4(tex2D(albedo, albedoUv));
        #endif
    #endif

    #if FLATALBEDO
        textureColor0 *= half4(flatColor);
    #endif

    #if MATERIAL_TEXTURE && ALPHATEST
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

    #if MATERIAL_DECAL
        half3 textureColor1 = half3(tex2D(decal, input.varTexCoord1).rgb);
    #endif

    #if MATERIAL_DETAIL
        half3 detailTextureColor = half3(tex2D(detail, input.varDetailTexCoord).rgb);
    #endif

    #if MATERIAL_DECAL && SETUP_LIGHTMAP
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

    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        float detailMaskSample = FP_A8(tex2D(detailMask, input.varTexCoord0.xy));
        detailMaskSample = lerp(detailMaskValueOffset, 1.0f, detailMaskSample);
        float3 detailNSMapSample = tex2D(detailNormalMap, input.varDetailNormalTexCoord).xyz;
        float detailSpecularValue = lerp(0.5f, detailNSMapSample.z, detailMaskSample);
        float2 detailNormalValue = detailNSMapSample.xy * 2.0f - 1.0f;
        detailNormalValue *= detailMaskSample * detailNormalScale;
    #endif

    // DRAW PHASE

    float specularSample = textureColor0.a;
    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        specularSample *= detailSpecularValue * 2.0;
    #endif

    // lookup normal from normal map, move from [0, 1] to  [-1, 1] range, normalize
    float3 normal = tex2D(normalmap, input.varTexCoord0).rgb * 2.0 - 1.0;
    normal.xy *= normalScale;
    normal = normalize(normal);
    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        normal = normalize(float3(normal.xy + detailNormalValue.xy, normal.z));
    #endif

    #if PIXEL_LIT
        float3 n = normalize(float3(
            dot(normal, input.tangentToView0),
            dot(normal, input.tangentToView1),
            dot(normal, input.tangentToView2)));
    #else
        float3 n = input.normal;
    #endif

    float3 color = float3(1.0, 1.0, 1.0);

    #if TILED_DECAL_MASK
        half maskSample = FP_A8(tex2D(decalmask, input.varTexCoord0));
        half4 tileColor = half4(tex2D(decaltexture, input.varDecalTileTexCoord).rgba * decalTileColor);
        color *= float3(textureColor0.rgb) + float3((tileColor.rgb - textureColor0.rgb) * tileColor.a * maskSample);
    #else
        color *= float3(textureColor0.rgb);
    #endif

    #if MATERIAL_DETAIL
        color *= float3(detailTextureColor.rgb * 2.0);
    #endif

    #if VERTEX_COLOR
        color *= float3(input.varVertexColor.xyz);
    #endif

    #if FLATCOLOR
        color *= flatColor.rgb;
    #endif

    output.gbuffer0.rgb = color;
    output.gbuffer0.a = 1.0;

    output.gbuffer1.rgb = n*0.5 + 0.5;
    output.gbuffer1.a = inSpecularity;

    output.gbuffer2.rgb = metalFresnelReflectance;

    #if NORMALIZED_BLINN_PHONG
        output.gbuffer2.a = specularSample * inGlossiness;
    #else
        output.gbuffer2.a = specularSample;
    #endif

    output.gbufferDepth.r = input.projectedPosition.z / input.projectedPosition.w * ndcToZMappingScale + ndcToZMappingOffset;
    output.gbufferDepth.gba = 0.0;

    return output;
}