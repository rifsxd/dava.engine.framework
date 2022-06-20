vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    output.position = float4(input.position.xyz, 1.0);
    output.texCoord = input.position.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;

    return output;
}
