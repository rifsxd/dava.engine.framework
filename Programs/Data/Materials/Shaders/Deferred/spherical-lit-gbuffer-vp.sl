#include "common.slh"
#include "materials-vertex-properties.slh"
#include "texture-coords-transform.slh"

vertex_in
{
    float3 position : POSITION;

    #if MATERIAL_TEXTURE
        float2 texcoord0 : TEXCOORD0;
    #endif

    #if MATERIAL_DECAL
        float2 texcoord1 : TEXCOORD1;
    #endif

    #if VERTEX_COLOR || VERTEX_DISTORTION
        float4 color0 : COLOR0;
    #endif

    #if SOFT_SKINNING
        float4 indices : BLENDINDICES;
        float4 weights : BLENDWEIGHT;
    #elif HARD_SKINNING
        float index : BLENDINDICES;
    #endif

    #if WIND_ANIMATION
        float flexibility : TEXCOORD5;
    #endif
};

vertex_out
{
    float4 position : SV_POSITION;

    float4 projectedPosition : TEXCOORD0;

    [lowp] half4 varVertexColor : COLOR1;

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float2 varTexCoord0 : TEXCOORD1;
    #endif

    #if MATERIAL_DECAL
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

    #if FLOWMAP
        [lowp] float3 varFlowData : TEXCOORD5; // For flowmap animations - xy next frame uv. z - frame time
    #endif
};

[auto][a] property float4x4 worldViewProjMatrix;
[auto][a] property float4x4 worldViewMatrix;

[auto][a] property float3 worldViewObjectCenter;
[auto][a] property float4x4 invViewMatrix;
[auto][a] property float3 boundingBoxSize;

#if SPHERICAL_HARMONICS_9
    [auto][sh] property float4 sphericalHarmonics[7] : "bigarray";
#elif SPHERICAL_HARMONICS_4
    [auto][sh] property float4 sphericalHarmonics[3] : "bigarray";
#else
    [auto][sh] property float4 sphericalHarmonics;
#endif

#if VERTEX_DISTORTION && VERTEX_SHADER_TEXTURE_FETCH_SUPPORTED
    uniform sampler2D vertexDistortionNoise; // Put a different perlin noise into each of rgb channels.

    [auto][a] property float4x4 viewProjMatrix;

    [material][a] property float vertexDistortionScale = 0.0f;
    [material][a] property float2 vertexDistortionVariationSpeed;
    [material][a] property float3 vertexDistortionWindVector;
#endif

#if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
    [material][a] property float2 detailNormalTexCoordOffset = float2(0, 0);
    [material][a] property float2 detailNormalTexCoordScale = float2(0, 0);
#endif

#if MATERIAL_DETAIL
    [material][a] property float2 detailTileCoordScale = float2(1.0, 1.0);
#endif

#if TEXTURE0_SHIFT_ENABLED
    [material][a] property float2 texture0Shift = float2(0,0);
#endif 

#if TEXTURE0_ANIMATION_SHIFT
    [material][a] property float2 tex0ShiftPerSecond = float2(0,0);
#endif

#if FLOWMAP
    [material][a] property float flowAnimSpeed = 0;
    [material][a] property float flowAnimOffset = 0;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out  output;

    #if FLOWMAP
        float flowSpeed = flowAnimSpeed;
        float flowOffset = flowAnimOffset;
        float scaledTime = globalTime * flowSpeed;
        float2 flowPhases = frac(float2(scaledTime, scaledTime+0.5))-float2(0.5, 0.5);
        float flowBlend = abs(flowPhases.x*2.0);
        output.varFlowData = float3(flowPhases * flowOffset, flowBlend);
    #endif

    #include "materials-vertex-processing.slh"

    #if SOFT_SKINNING || HARD_SKINNING
        float3 eyeCoordsPosition = mul(skinnedPosition, worldViewMatrix).xyz; // view direction in view space
    #else
        // view direction in view space
        float3 eyeCoordsPosition = mul(float4(input.position.xyz, 1.0), worldViewMatrix).xyz;
    #endif

    #if VERTEX_COLOR
        output.varVertexColor = half4(input.color0);
    #endif

    #if SPHERICAL_HARMONICS_4 || SPHERICAL_HARMONICS_9
        float3 sphericalLightFactor = 0.282094 * sphericalHarmonics[0].xyz;
        float3x3 invViewMatrix3 = float3x3(float3(invViewMatrix[0].xyz), float3(invViewMatrix[1].xyz), float3(invViewMatrix[2].xyz));
        float3 normal = mul((eyeCoordsPosition - worldViewObjectCenter), invViewMatrix3);
        normal /= boundingBoxSize;
        float3 n = normalize(normal);

        float3x3 shMatrix = float3x3(float3(sphericalHarmonics[0].w, sphericalHarmonics[1].xy),
                                     float3(sphericalHarmonics[1].zw, sphericalHarmonics[2].x),
                                     float3(sphericalHarmonics[2].yzw));
        sphericalLightFactor += 0.325734 * mul(float3(n.y, n.z, n.x), shMatrix);

        #if SPHERICAL_HARMONICS_9
            sphericalLightFactor += (0.273136 * (n.y * n.x)) * float3(sphericalHarmonics[3].xyz);
            sphericalLightFactor += (0.273136 * (n.y * n.z)) * float3(sphericalHarmonics[3].w,  sphericalHarmonics[4].xy);
            sphericalLightFactor += (0.078847 * (3.0 * n.z * n.z - 1.0)) * float3(sphericalHarmonics[4].zw, sphericalHarmonics[5].x);
            sphericalLightFactor += (0.273136 * (n.z * n.x))  * float3(sphericalHarmonics[5].yzw);
            sphericalLightFactor += (0.136568 * (n.x * n.x - n.y * n.y)) * float3(sphericalHarmonics[6].xyz);
        #endif
    #else
        float3 sphericalLightFactor = 0.282094 * sphericalHarmonics.xyz;
    #endif

    #if VERTEX_COLOR
        output.varVertexColor.xyz = half3(input.color0.xyz) * half3(sphericalLightFactor * 2.0);
    #else
        output.varVertexColor.xyz = half3(sphericalLightFactor * 2.0);
    #endif

    output.varVertexColor.w = half(1.0);

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        output.varTexCoord0.xy = ApplyTex0CoordsTransform(input.texcoord0);
    #endif

    #if MATERIAL_TEXTURE
        #if TEXTURE0_SHIFT_ENABLED
            output.varTexCoord0.xy += texture0Shift;
        #endif

        #if TEXTURE0_ANIMATION_SHIFT
            output.varTexCoord0.xy += frac(tex0ShiftPerSecond * globalTime);
        #endif
    #endif

    #if TILED_DECAL_MASK
        float2 resDecalTexCoord = output.varTexCoord0.xy * decalTileCoordScale;    
        #if TILED_DECAL_TRANSFORM
            #if HARD_SKINNING
                resDecalTexCoord = ApplyTex1CoordsTransformHardSkin(resDecalTexCoord, input.index);
            #elif !SOFT_SKINNING
                resDecalTexCoord = ApplyTex1CoordsTransform(resDecalTexCoord);
            #endif
        #endif
        output.varDecalTileTexCoord = resDecalTexCoord;
    #endif

    #if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
        output.varDetailNormalTexCoord = output.varTexCoord0.xy * detailNormalTexCoordScale + detailNormalTexCoordOffset;
    #endif

    #if MATERIAL_DETAIL
        output.varDetailTexCoord = output.varTexCoord0.xy * detailTileCoordScale;
    #endif

    #if MATERIAL_DECAL
        output.varTexCoord1 = input.texcoord1.xy;
    #endif

    #if FORCE_2D_MODE
        output.position.z=0.0;
    #endif

    output.projectedPosition = output.position;

    return output;
}
