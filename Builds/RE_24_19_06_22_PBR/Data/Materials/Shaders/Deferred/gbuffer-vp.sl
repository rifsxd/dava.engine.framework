#include "common.slh"
#include "materials-vertex-properties.slh"
#include "texture-coords-transform.slh"

vertex_in
{
    float3 position : POSITION;

    #if MATERIAL_TEXTURE
        float2 texcoord0 : TEXCOORD0;
    #endif

    #if MATERIAL_DECAL || (MATERIAL_LIGHTMAP  && VIEW_DIFFUSE) || ALPHA_MASK
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

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float2 varTexCoord0 : TEXCOORD1;
    #endif

    #if MATERIAL_DECAL || (MATERIAL_LIGHTMAP  && VIEW_DIFFUSE) || ALPHA_MASK
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

    #if VERTEX_COLOR
        [lowp] half4 varVertexColor : COLOR1;
    #endif

    #if FLOWMAP
        [lowp] float3 varFlowData : TEXCOORD5; // For flowmap animations - xy next frame uv. z - frame time
    #endif
};

////////////////////////////////////////////////////////////////////////////////
// properties

[auto][a] property float4x4 worldViewProjMatrix;

#if MATERIAL_LIGHTMAP && VIEW_DIFFUSE && !SETUP_LIGHTMAP
    [material][a] property float2 uvOffset = float2(0,0);
    [material][a] property float2 uvScale = float2(0,0);
#endif

#if NORMAL_DETAIL && ALLOW_NORMAL_DETAIL
    [material][a] property float2 detailNormalTexCoordOffset = float2(0, 0);
    [material][a] property float2 detailNormalTexCoordScale = float2(0, 0);
#endif

#if MATERIAL_DETAIL
    [material][a] property float2 detailTileCoordScale = float2(1.0, 1.0);
#endif

#if TEXTURE0_SHIFT_ENABLED
    [material][a] property float2 texture0Shift = float2(0, 0);
#endif 
#if TEXTURE0_ANIMATION_SHIFT
    [material][a] property float2 tex0ShiftPerSecond = float2(0, 0);
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

    #if VERTEX_COLOR
        output.varVertexColor = half4(input.color0);
    #endif

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        output.varTexCoord0.xy = ApplyTex0CoordsTransform(input.texcoord0);
        #if PARTICLES_PERSPECTIVE_MAPPING
            output.varTexCoord0.z = input.texcoord5.z;
        #endif
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

    #if MATERIAL_DECAL || (MATERIAL_LIGHTMAP  && VIEW_DIFFUSE) || ALPHA_MASK
        #if MATERIAL_LIGHTMAP && VIEW_DIFFUSE && !SETUP_LIGHTMAP
            output.varTexCoord1 = uvScale*input.texcoord1.xy + uvOffset;
        #else
            output.varTexCoord1 = input.texcoord1.xy;
        #endif
    #endif

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif

    output.projectedPosition = output.position;

    return output;
}
