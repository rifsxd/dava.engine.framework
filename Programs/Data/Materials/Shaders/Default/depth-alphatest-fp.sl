#include "common.slh"
#include "blending.slh"
#include "lighting.slh"

fragment_in
{
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

fragment_out
{
    float4 color : SV_TARGET0;
};

#if MATERIAL_TEXTURE
    uniform sampler2D albedo;
#endif

#if ALPHA_MASK
    uniform sampler2D alphamask;
#endif

#if MATERIAL_TEXTURE && ALPHATEST && ALPHATESTVALUE
    [material][a] property float alphatestThreshold = 0.0;
#endif

#if MATERIAL_TEXTURE && ALPHASTEPVALUE && ALPHABLEND
    [material][a] property float alphaStepValue = 0.5;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    // FETCH PHASE
    half4 textureColor0 = half4(1.0, 0.0, 0.0, 1.0);
    #if MATERIAL_TEXTURE
        #if ALPHATEST || ALPHABLEND
            float2 albedoUv = input.varTexCoord0.xy;
            textureColor0 = half4(tex2D(albedo, albedoUv));

            #if ALPHA_MASK
                textureColor0.a *= FP_A8(tex2D(alphamask, input.varTexCoord1));
            #endif

          #else
            textureColor0.rgb = half3(tex2D(albedo, input.varTexCoord0).rgb);
        #endif
    #endif

    #if MATERIAL_TEXTURE
        #if ALPHATEST
            float alpha = textureColor0.a;
            #if VERTEX_COLOR
                alpha *= float(input.varVertexColor.a);
            #endif
            #if ALPHATESTVALUE
                if( alpha < alphatestThreshold ) discard;
            #else
                if( alpha < 0.5 ) discard;
            #endif
        #endif
    #endif

    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}
