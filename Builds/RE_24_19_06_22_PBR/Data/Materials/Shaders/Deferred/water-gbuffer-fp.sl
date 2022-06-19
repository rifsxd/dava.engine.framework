#include "common.slh"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;

    float2 uv : TEXCOORD1;
    float2 uv1 : TEXCOORD2;
    float3 cameraToPointInTangentSpace : TEXCOORD3;
    half3 tbnToWorld0 : TEXCOORD4;
    half3 tbnToWorld1 : TEXCOORD5;
    half3 tbnToWorld2 : TEXCOORD6;
};

fragment_out
{
    float4 gbufferDepth : SV_TARGET0;
    float4 gbuffer0 : SV_TARGET1;
    float4 gbuffer1 : SV_TARGET2;
    float4 gbuffer2 : SV_TARGET3;
};

uniform sampler2D normalmap;
uniform samplerCUBE cubemap;

[material][instance] property float3 reflectionTintColor = float3(1, 1, 1);
[material][instance] property float fresnelBias = 0;
[material][instance] property float fresnelPow = 0.0;

inline float FresnelShlick(float _NdotL, float _fresnelBias, float _fresnelPow)
{
    return _fresnelBias + (1.0 - _fresnelBias) * pow(1.0 - _NdotL, _fresnelPow);
}

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float2 varTexCoord0 = input.uv;
    float2 varTexCoord1 = input.uv1;

    float3 normal0 = tex2D(normalmap, varTexCoord0).rgb;
    float3 normal1 = tex2D(normalmap, varTexCoord1).rgb;

    float3 normal = normalize(normal0 + normal1 - 1.0); //same as * 2 -2

    //compute fresnel
    float3 cameraToPointInTangentSpaceNorm = float3(normalize(input.cameraToPointInTangentSpace));
    float lambertFactor = max(dot(-cameraToPointInTangentSpaceNorm, normal), 0.0);
    float fresnel = FresnelShlick(lambertFactor, fresnelBias, fresnelPow);

    float3x3 tbnToWorldMatrix = float3x3(float3(input.tbnToWorld0), float3(input.tbnToWorld1), float3(input.tbnToWorld2));
    float3 reflectionVectorInTangentSpace = reflect(cameraToPointInTangentSpaceNorm, normal);
    reflectionVectorInTangentSpace.z = abs(reflectionVectorInTangentSpace.z); //prevent reflection through surface
    float3 reflectionVectorInWorldSpace = mul(reflectionVectorInTangentSpace, tbnToWorldMatrix);
    float3 reflectionColor = texCUBE(cubemap, reflectionVectorInWorldSpace).rgb * reflectionTintColor;

    output.gbuffer0 = float4(reflectionColor, 0.0);
    output.gbuffer1 = 0.0;
    output.gbuffer2 = 0.0;
    output.gbufferDepth.r = input.projectedPosition.z / input.projectedPosition.w * ndcToZMappingScale + ndcToZMappingOffset;
    output.gbufferDepth.gba = 0.0;

    return output;
}

