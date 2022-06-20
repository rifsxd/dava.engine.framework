#include "common.slh"
#include "blending.slh"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
    [lowp] half4 varVertexColor : COLOR1;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D albedo;

#if ALPHATEST && ALPHATESTVALUE
    [material][a] property float alphatestThreshold = 0.0;
#endif

#if ALPHASTEPVALUE && ALPHABLEND
    [material][a] property float alphaStepValue = 0.5;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    half4 textureColor0 = half4(1.0, 0.0, 0.0, 1.0);

    #if ALPHATEST || ALPHABLEND
        textureColor0 = half4(tex2D(albedo, input.varTexCoord0.xy));
        float alpha = textureColor0.a * float(input.varVertexColor.a);

        #if ALPHATESTVALUE
            if(alpha < alphatestThreshold) discard;
        #else
            if(alpha < 0.5) discard;
        #endif
    #endif

    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}
