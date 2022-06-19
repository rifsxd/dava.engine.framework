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

#if VERTEX_FOG
    #include "vp-fog-props.slh"

    [auto][a] property float4x4 invProjMatrix;
    [auto][a] property float4x4 invViewMatrix;
    [auto][a] property float4 lightPosition0;
    [auto][a] property float3 cameraPosition;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float2 texCoord = input.texcoord0;
    float4 gbuffer0Sample = tex2D(gbuffer0, texCoord);
    float3 baseColor = gbuffer0Sample.rgb;

    output.color.rgb = baseColor;
    output.color.a = 1.0;

    #if VERTEX_FOG
        float depthSample = tex2D(depthBuffer, texCoord).r;
        float depth = (depthSample - ndcToZMappingOffset) / ndcToZMappingScale;
        float2 xyNDC = input.positionNDC;
        float4 intermediatePosition = mul(float4(xyNDC, depth, 1.0), invProjMatrix);
        float3 P = intermediatePosition.xyz / intermediatePosition.w;
        float3 L = lightPosition0.xyz - P * lightPosition0.w;

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
