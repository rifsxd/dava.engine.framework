#include "common.slh"
#include "blending.slh"

fragment_in
{
    float2 positionNDC : TEXCOORD0;
    float2 texcoord0 : TEXCOORD1;
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
[auto][a] property float3 lightColor0;
[auto][a] property float3 lightAmbientColor0;

#if VERTEX_FOG
    #include "vp-fog-props.slh"
    [auto][a] property float3 cameraPosition;
#endif

float3 UnpackNormal(float3 normalSample)
{
    return normalize(normalSample * 2 - 1);
}

inline float3 FresnelShlick(float NdotL, float3 Cspec)
{
    float expf = 5.0;
    return Cspec + (1.0 - Cspec) * (pow(1.0 - NdotL, expf));
}

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float2 texCoord = input.texcoord0;
    float4 gbuffer0Sample = float4(tex2D(gbuffer0, texCoord));
    float3 baseColor = gbuffer0Sample.rgb;

    float depthSample = tex2D(depthBuffer, texCoord).r;
    float depth = (depthSample - ndcToZMappingOffset) / ndcToZMappingScale;
    float2 xyNDC = input.positionNDC;
    float4 intermediatePosition = mul(float4(xyNDC, depth, 1.0), invProjMatrix);
    float3 P = intermediatePosition.xyz / intermediatePosition.w;
    float3 eyePosVS = 0.0; // in view space eye is at origin
    float3 V = normalize(eyePosVS - P);
    float3 L = lightPosition0.xyz - P * lightPosition0.w;

    if(gbuffer0Sample.a != 1.0)
    {
        output.color.rgb = gbuffer0Sample.rgb;
    }
    else
    {
        float4 gbuffer1Sample = float4(tex2D(gbuffer1, texCoord));
        float4 gbuffer2Sample = float4(tex2D(gbuffer2, texCoord));

        float3 N = UnpackNormal(gbuffer1Sample.rgb);
        float specularitySample = gbuffer1Sample.a;

        float3 metalFresnelReflectance = gbuffer2Sample.rgb;
        float glossinessSample = gbuffer2Sample.a;

        #if DISTANCE_ATTENUATION
            float intensity = lightIntensity0;
            float distanceToLight = length(L);
            intensity /= (distanceToLight * distanceToLight); // use inverse distance for distance attenuation
        #endif

        L = normalize(L);

        float3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float LdotH = max(dot(L, H), 0.0);
        float NdotV = max(dot(N, V), 0.0);

        float3 diffuse = NdotL / _PI;
        float3 fresnelOut = FresnelShlick(NdotV, metalFresnelReflectance);

        float glossPower = pow(5000.0, glossinessSample);

        float specCutoff = NdotL;
        float specularNorm = (glossPower + 2.0) / 8.0;
        float specularNormalized = specularNorm * pow(NdotH, glossPower) * specCutoff * specularitySample;

        float3 specular = specularNormalized * fresnelOut;

        float3 color = baseColor * (lightAmbientColor0 + diffuse * lightColor0) + specular * lightColor0;

        output.color.rgb = color;
        output.color.a = 1.0;
    }

    #if VERTEX_FOG
        float3 worldPosition = mul(float4(P, 1.0), invViewMatrix).xyz;
        #define FOG_eye_position cameraPosition
        #define FOG_view_position P
        #define FOG_world_position worldPosition

        #if FOG_ATMOSPHERE
            #define FOG_to_light_dir L
        #endif

        #include "vp-fog-math.slh"

        float4 varFog = FOG_result;

        output.color.rgb = lerp(output.color.rgb, varFog.rgb, varFog.a);
    #endif
    

    return output;
}