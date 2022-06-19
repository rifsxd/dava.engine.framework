#include "common.slh"
#include "blending.slh"
#include "lighting.slh"

blending { src=one dst=one }

fragment_in
{
    float4 positionCS : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D depthBuffer;
uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;

[auto][a] property float4x4 invProjMatrix;
[auto][a] property float4x4 invViewMatrix;
[auto][a] property float4 lightPosition0;
[auto][a] property float4 lightColor0;

#if POINT_LIGHT_AFFECTS_UNLIT
    [auto][a] property float2 renderTargetSize;
#endif

float3 UnpackNormal(float2 normalSample)
{
    float2 normalXY = normalSample * 2.0 - 1.0;
    float normalZ = sqrt(1 - dot(normalXY, normalXY));
    return float3(normalXY, normalZ);
}

inline float3 FresnelShlick(float NdotL, float3 Cspec)
{
    float expf = 5.0;
    return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, expf));
}

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float3 positionNDC = input.positionCS.xyz / input.positionCS.w;
    float2 xyNDC = positionNDC.xy;
    float2 texCoord = positionNDC.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    float4 gbuffer0Sample = tex2D(gbuffer0, texCoord);
    float3 baseColor = gbuffer0Sample.rgb;

    float depthSample = tex2D(depthBuffer, texCoord).r;
    float depthNDC = (depthSample - ndcToZMappingOffset) / ndcToZMappingScale;
    float4 intermediatePosition = mul(float4(xyNDC, depthNDC, 1.0), invProjMatrix);
    float3 P = intermediatePosition.xyz / intermediatePosition.w;
    float3 eyePosVS = 0.0; // in view space eye is at origin
    float3 V = normalize(eyePosVS - P);

    if(gbuffer0Sample.a != 1.0)
    {
        #if POINT_LIGHT_AFFECTS_UNLIT
            float2 tc1 = texCoord + float2(1.0 / renderTargetSize.x, 0);
            float2 tc2 = texCoord + float2(0, 1.0 / renderTargetSize.y);

            float d1 = tex2D(depthBuffer, tc1).r;
            float d2 = tex2D(depthBuffer, tc2).r;

            float d1NDC = (d1 - ndcToZMappingOffset) / ndcToZMappingScale;
            float d2NDC = (d2 - ndcToZMappingOffset) / ndcToZMappingScale;

            float2 tc1NDC = (tc1 - ndcToUvMapping.zw) / ndcToUvMapping.xy;
            float2 tc2NDC = (tc2 - ndcToUvMapping.zw) / ndcToUvMapping.xy;

            float4 ip1 = mul(float4(tc1NDC, d1NDC, 1.0), invProjMatrix);
            float4 ip2 = mul(float4(tc2NDC, d2NDC, 1.0), invProjMatrix);

            float3 P1 = ip1.xyz / ip1.w;
            float3 P2 = ip2.xyz / ip2.w;

            float3 N = -normalize(cross(P1 - P, P2 - P));

            float3 diffuse = ApplyLight(lightPosition0, lightColor0, P, N);

            output.color.rgb = baseColor*diffuse;
            output.color.a = 1.0;
        #else
            output.color.rgb = 0.0;
            output.color.a = 1.0;
        #endif
    }
    else
    {
        float4 gbuffer1Sample = tex2D(gbuffer1, texCoord);

        float3 N = UnpackNormal(gbuffer1Sample.rg);

        float3 diffuse = ApplyLight(lightPosition0, lightColor0, P, N);

        float3 color = baseColor * diffuse;

        output.color.rgb = color;
        output.color.a = 1.0;
    }

    return output;
}