#include "common.slh"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
    #if TILEMASK
        float2 texCoordTiled : TEXCOORD2;
    #endif
};

fragment_out
{
    float4 gbufferDepth : SV_TARGET0;
    float4 gbuffer0 : SV_TARGET1;
    float4 gbuffer1 : SV_TARGET2;
    float4 gbuffer2 : SV_TARGET3;
};

uniform sampler2D tileTexture0;
uniform sampler2D tileMask;
uniform sampler2D colorTexture;

#if LANDSCAPE_TOOL
    uniform sampler2D toolTexture;
#endif

[material][instance] property float3 tileColor0 = float3(1, 1, 1);
[material][instance] property float3 tileColor1 = float3(1, 1, 1);
[material][instance] property float3 tileColor2 = float3(1, 1, 1);
[material][instance] property float3 tileColor3 = float3(1, 1, 1);

#if LANDSCAPE_CURSOR
    uniform sampler2D cursorTexture;
    [material][instance] property float4 cursorCoordSize = float4(0, 0, 1, 1);
#endif

fragment_out fp_main( fragment_in input )
{
    fragment_out output;

    float4 tileColor = tex2D(tileTexture0, input.texCoordTiled);
    float4 mask = tex2D(tileMask, input.texCoord);
    float4 color = tex2D(colorTexture, input.texCoord);

    float3 color3 = (tileColor.r * mask.r * tileColor0.rgb + 
                     tileColor.g * mask.g * tileColor1.rgb + 
                     tileColor.b * mask.b * tileColor2.rgb + 
                     tileColor.a * mask.a * tileColor3.rgb ) * color.rgb * 2.0;
    float4 outColor = float4(color3, 1.0);

    #if LANDSCAPE_TOOL
        float4 toolColor = tex2D( toolTexture, input.texCoord );
        #if LANDSCAPE_TOOL_MIX
            outColor.rgb = (outColor.rgb + toolColor.rgb) / 2.0;
        #else
            outColor.rgb *= 1.0 - toolColor.a;
            outColor.rgb += toolColor.rgb * toolColor.a;
        #endif
    #endif

    #if LANDSCAPE_CURSOR
        float2 cursorCoord = (input.texCoord - cursorCoordSize.xy) / cursorCoordSize.zw + float2(0.5, 0.5);
        float4 cursorColor = tex2D(cursorTexture, cursorCoord);
        outColor.rgb *= 1.0 - cursorColor.a;
        outColor.rgb += cursorColor.rgb * cursorColor.a;
    #endif

    output.gbufferDepth.r = input.projectedPosition.z / input.projectedPosition.w * ndcToZMappingScale + ndcToZMappingOffset;
    output.gbuffer0 = float4(outColor.rgb, 0);

    return output;
}
