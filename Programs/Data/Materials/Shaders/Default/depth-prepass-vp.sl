#include "common.slh"
#define DRAW_DEPTH_ONLY 1
#include "materials-vertex-properties.slh"

vertex_in
{
    float3 position : POSITION;

    #if VERTEX_DISTORTION
        float2 texcoord0 : TEXCOORD0;
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
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    #include "materials-vertex-processing.slh"
    output.projectedPosition = output.position;
    return output;
}
