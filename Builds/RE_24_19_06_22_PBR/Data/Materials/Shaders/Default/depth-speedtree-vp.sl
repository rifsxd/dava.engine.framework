#include "common.slh"

vertex_in
{
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
    float4 color0 : COLOR0;

    float4 pivot : TEXCOORD4;

    #if WIND_ANIMATION
        float flexibility : TEXCOORD5;
        float2 angleSinCos : TEXCOORD6;
    #endif
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;
    [lowp] half4 varVertexColor : COLOR1;
};

////////////////////////////////////////////////////////////////////////////////
// properties

[auto][a] property float4x4 worldViewProjMatrix;

[auto][a] property float3 worldScale;
[auto][a] property float4x4 projMatrix;
[auto][a] property float4x4 worldViewMatrix;

#if CUT_LEAF
    [material][a] property float cutDistance = 1.0;
#endif

#if !SPHERICAL_LIT  //legacy for old tree lighting
    [material][a] property float4 treeLeafColorMul = float4(0.5,0.5,0.5,0.5) ;
    [material][a] property float treeLeafOcclusionOffset = 0.0 ;
    [material][a] property float treeLeafOcclusionMul = 0.5 ;
#endif

#if WIND_ANIMATION
    [auto][a] property float2 leafOscillationParams; //x: A*sin(T); y: A*cos(T);
#endif

#if SPHERICAL_LIT
    [auto][a] property float speedTreeLightSmoothing;
#endif

#if WIND_ANIMATION
    [auto][a] property float2 trunkOscillationParams;
#endif

#if WAVE_ANIMATION || TEXTURE0_ANIMATION_SHIFT
    [auto][a] property float globalTime;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out  output;

    float3 position = lerp(input.position.xyz, input.pivot.xyz, input.pivot.w);
    float3 billboardOffset = input.position.xyz - position.xyz;

    #if CUT_LEAF
        float pivotDistance = dot(position.xyz, float3(worldViewMatrix[0].z, worldViewMatrix[1].z, worldViewMatrix[2].z)) + worldViewMatrix[3].z;
        billboardOffset *= step(-cutDistance, pivotDistance);
    #endif

    #if WIND_ANIMATION
        //inAngleSinCos:          x: cos(T0);  y: sin(T0);
        //leafOscillationParams:  x: A*sin(T); y: A*cos(T);
        float3 windVectorFlex = float3(trunkOscillationParams * input.flexibility, 0.0);
        position += windVectorFlex;

        float2 SinCos = input.angleSinCos * leafOscillationParams; //vec2(A*sin(t)*cos(t0), A*cos(t)*sin(t0))
        float sinT = SinCos.x + SinCos.y;     //sin(t+t0)*A = sin*cos + cos*sin
        float cosT = 1.0 - 0.5 * sinT * sinT; //cos(t+t0)*A = 1 - 0.5*sin^2

        float4 SinCosT = float4(sinT, cosT, cosT, sinT); //temp vec for mul
        float4 offsetXY = float4(billboardOffset.x, billboardOffset.y, billboardOffset.x, billboardOffset.y); //temp vec for mul
        float4 rotatedOffsetXY = offsetXY * SinCosT; //vec4(x*sin, y*cos, x*cos, y*sin)

        billboardOffset.x = rotatedOffsetXY.z - rotatedOffsetXY.w; //x*cos - y*sin
        billboardOffset.y = rotatedOffsetXY.x + rotatedOffsetXY.y; //x*sin + y*cos
    #endif

    float4 eyeCoordsPosition4 = mul(float4(position, 1.0), worldViewMatrix) + float4(worldScale * billboardOffset, 0.0);

    output.position = mul(eyeCoordsPosition4, projMatrix);

    float3 eyeCoordsPosition = eyeCoordsPosition4.xyz;

    output.varVertexColor = half4(input.color0);
    output.varTexCoord0.xy = input.texcoord0;

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif

    return output;
}
