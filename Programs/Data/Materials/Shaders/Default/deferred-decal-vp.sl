#include "common.slh"
#include "vp-fog-props.slh"

vertex_in
{
    [vertex] float3 position : POSITION;
    [instance] float4 worldMatrix0 : TEXCOORD0;
    [instance] float4 worldMatrix1 : TEXCOORD1;
    [instance] float4 worldMatrix2 : TEXCOORD2;
    [instance] float4 invWorldMatrix0 : TEXCOORD3;
    [instance] float4 invWorldMatrix1 : TEXCOORD4;
    [instance] float4 invWorldMatrix2 : TEXCOORD5;
    [instance] float4 instanceOpacity : TEXCOORD6;
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 projectedPosition : COLOR0;
    float4 invWorldMatrix0 : TEXCOORD0;
    float4 invWorldMatrix1 : TEXCOORD1;
    float4 invWorldMatrix2 : TEXCOORD2;
    float4 instanceOpacity : TEXCOORD3;

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD4;
    #endif


};

[auto][a] property float4x4 viewProjMatrix;
[auto][a] property float nearFadeDistance;
[auto][a] property float farFadeDistance;
[auto][a] property float3 cameraPosition;

#if VERTEX_FOG
    [auto][a] property float4x4 viewMatrix;
#endif

#if VERTEX_FOG && FOG_ATMOSPHERE
    [auto][a] property float4 lightPosition0;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float4x4 worldMatrix = float4x4(
        float4(input.worldMatrix0.x,  input.worldMatrix1.x,  input.worldMatrix2.x, 0.0),
        float4(input.worldMatrix0.y,  input.worldMatrix1.y,  input.worldMatrix2.y, 0.0),
        float4(input.worldMatrix0.z,  input.worldMatrix1.z,  input.worldMatrix2.z, 0.0),
        float4(input.worldMatrix0.w,  input.worldMatrix1.w,  input.worldMatrix2.w, 1.0)
    );

    float4 worldPosition = mul(float4(input.position.xyz, 1.0), worldMatrix);
    output.position = mul(worldPosition, viewProjMatrix);

    output.projectedPosition = output.position;

    output.invWorldMatrix0 = input.invWorldMatrix0;
    output.invWorldMatrix1 = input.invWorldMatrix1;
    output.invWorldMatrix2 = input.invWorldMatrix2;

    float dist = clamp(length(cameraPosition - worldPosition.xyz), nearFadeDistance, farFadeDistance);
    output.instanceOpacity =  (1.0 - (dist - nearFadeDistance) / max(farFadeDistance - nearFadeDistance, 0.0001)) * input.instanceOpacity;

    #if VERTEX_FOG
        float3 FOG_view_position = mul(worldPosition, viewMatrix).xyz;
        #define FOG_to_light_dir lightPosition0.xyz
        #define FOG_eye_position cameraPosition
        #define FOG_in_position input.position
        #define FOG_world_position worldPosition
        #include "vp-fog-math.slh"
        output.varFog = half4(FOG_result);
    #endif

    return output;
}