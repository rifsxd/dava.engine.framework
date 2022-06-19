#include "common.slh"

vertex_in
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;

    float2 uv0 : TEXCOORD0;
};

vertex_out
{
    float4 position : SV_POSITION;

    float4 projectedPosition : TEXCOORD0;

    float2 uv : TEXCOORD1;
    float2 uv1 : TEXCOORD2;
    float3 cameraToPointInTangentSpace : TEXCOORD3;
    half3 tbnToWorld0 : TEXCOORD4;
    half3 tbnToWorld1 : TEXCOORD5;
    half3 tbnToWorld2 : TEXCOORD6;
};

[auto][instance] property float4x4 worldViewProjMatrix;
[auto][instance] property float4x4 worldViewInvTransposeMatrix;
[auto][instance] property float4x4 worldViewMatrix;

[auto][instance] property float globalTime;

[material][instance] property float2 normal0ShiftPerSecond = float2(0, 0);
[material][instance] property float2 normal1ShiftPerSecond = float2(0, 0);
[material][instance] property float normal0Scale = 0;
[material][instance] property float normal1Scale = 0;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    //position
    float4 inPosition = float4(input.position.xyz, 1.0);
    float4 resPosition = mul(inPosition, worldViewProjMatrix);

    output.position = resPosition;

    //texcoords
    float2 inTexCoord0 = input.uv0;
    output.uv = inTexCoord0 * normal0Scale + frac(normal0ShiftPerSecond * globalTime);
    output.uv1 = float2(inTexCoord0.x + inTexCoord0.y, inTexCoord0.y - inTexCoord0.x) * normal1Scale + frac(normal1ShiftPerSecond * globalTime);

    //shading requirements
    float3 inNormal = input.normal;
    float3 inTangent = input.tangent;

    float3 eyeCoordsPosition = mul(inPosition, worldViewMatrix).xyz;

    float3 n = normalize(mul(float4(inNormal, 0.0), worldViewInvTransposeMatrix ).xyz);
    float3 t = normalize(mul(float4(inTangent, 0.0), worldViewInvTransposeMatrix ).xyz);
    float3 b = cross (n, t);

    float3 v;
    v.x = dot(eyeCoordsPosition, t);
    v.y = dot(eyeCoordsPosition, b);
    v.z = dot(eyeCoordsPosition, n);
    output.cameraToPointInTangentSpace = v;

    output.tbnToWorld0 = half3(inTangent);
    output.tbnToWorld1 = half3(cross(inNormal, inTangent));
    output.tbnToWorld2 = half3(inNormal);

    output.projectedPosition = output.position;

    return output;
}
