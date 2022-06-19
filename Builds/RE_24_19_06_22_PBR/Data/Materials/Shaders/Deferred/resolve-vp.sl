#include "common.slh"

#ensuredefined INVERT_PROJECTION 0

vertex_in
{
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 positionNDC : TEXCOORD0;
    float2 texcoord0 : TEXCOORD1;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out  output;

    output.position = float4(input.position.xyz, 1.0);
    output.positionNDC = input.position.xy;
    output.texcoord0 = input.texcoord0;

#if (INVERT_PROJECTION)
    output.texcoord0.y = 1.0 - input.texcoord0.y;
#endif

    return output;
}
