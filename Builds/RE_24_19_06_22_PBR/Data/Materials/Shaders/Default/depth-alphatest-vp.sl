#include "common.slh"
#define DRAW_DEPTH_ONLY 1
#include "materials-vertex-properties.slh"

vertex_in
{
    float3 position : POSITION;

    #if MATERIAL_TEXTURE
        float2 texcoord0 : TEXCOORD0;
    #endif

    #if ALPHA_MASK
        float2 texcoord1 : TEXCOORD1;
    #endif

    #if VERTEX_COLOR
        float4 color0 : COLOR0;
    #endif

    #if SOFT_SKINNING
        float4 index : BLENDINDICES;
        float4 weight : BLENDWEIGHT;
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

    #if MATERIAL_TEXTURE
        float2 varTexCoord0 : TEXCOORD0;
    #endif

    #if ALPHA_MASK
        float2 varTexCoord1 : TEXCOORD1;
    #endif

    #if VERTEX_COLOR
        [lowp] half4 varVertexColor : COLOR1;
    #endif
};

////////////////////////////////////////////////////////////////////////////////
// properties

[auto][a] property float4x4 worldViewProjMatrix;

#if TEXTURE0_SHIFT_ENABLED
    [material][a] property float2 texture0Shift = float2(0, 0);
#endif
#if TEXTURE0_ANIMATION_SHIFT
    [material][a] property float2 tex0ShiftPerSecond = float2(0, 0);
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out  output;

    #include "materials-vertex-processing.slh"

    #if PARTICLES_FLOWMAP_ANIMATION
        float flowOffset = input.texcoord2.w;
        output.varParticleFlowTexCoord = input.texcoord2.xy;
        output.varFlowData.xy = input.flowmapCrossfadeData.xy;
        output.varFlowData.z = flowOffset;
    #endif

    #if VERTEX_COLOR
        output.varVertexColor = half4(input.color0);
    #endif

    #if MATERIAL_TEXTURE
        output.varTexCoord0.xy = input.texcoord0;
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

    #if ALPHA_MASK
        output.varTexCoord1 = input.texcoord1.xy;
    #endif

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif

    return output;
}
