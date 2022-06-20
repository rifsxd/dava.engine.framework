#include "common.slh"
#include "materials-vertex-properties.slh"
#include "texture-coords-transform.slh"

vertex_in
{
    float3 position : POSITION;
    float3 normal : NORMAL;

    #if MATERIAL_TEXTURE
        float2 texcoord0 : TEXCOORD0;
    #endif

    #if MATERIAL_DECAL || ALPHA_MASK
        float2 texcoord1 : TEXCOORD1;
    #endif

    #if VERTEX_COLOR || VERTEX_DISTORTION
        float4 color0 : COLOR0;
    #endif

    #if PIXEL_LIT
        float3 tangent : TANGENT;
        float3 binormal : BINORMAL;
    #endif

    #if SOFT_SKINNING
        float4 indices : BLENDINDICES;
        float4 weights : BLENDWEIGHT;
    #elif HARD_SKINNING
        float index : BLENDINDICES;
    #endif
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 projectedPosition : TEXCOORD0;

    #if MATERIAL_TEXTURE || TILED_DECAL_MASK
        float2 varTexCoord0 : TEXCOORD1;
    #endif

    #if MATERIAL_DECAL || ALPHA_MASK
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

    #if PIXEL_LIT
        float3 tangentToView0 : NORMAL1; // here we are storing transposed (T, B, N) matrix
        float3 tangentToView1 : NORMAL2; // used for transforming normal from tangent space to view space
        float3 tangentToView2 : NORMAL3;
    #else
        float3 normal : NORMAL0;
    #endif

    #if VERTEX_COLOR
        [lowp] half4 varVertexColor : COLOR1;
    #endif
};

[auto][a] property float4x4 worldViewProjMatrix;
[auto][a] property float4x4 worldViewInvTransposeMatrix;

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

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    #include "materials-vertex-processing.slh"

    #if PIXEL_LIT
        float3 inNormal = input.normal;
        float3 inTangent = input.tangent;
        float3 inBinormal = input.binormal;

        #if SOFT_SKINNING
            float3x3 tbn = SoftSkinnedTBN(inTangent, inBinormal, inNormal, input.indices, input.weights);
            float3 t = tbn[0];
            float3 b = tbn[1];
            float3 n = tbn[2];
        #elif HARD_SKINNING
            float3x3 tbn = HardSkinnedTBN(inTangent, inBinormal, inNormal, input.index);
            float3 t = tbn[0];
            float3 b = tbn[1];
            float3 n = tbn[2];
        #else
            float3 t = inTangent;
            float3 b = inBinormal;
            float3 n = inNormal;
        #endif

        t = normalize(mul(float4(t, 0.0), worldViewInvTransposeMatrix).xyz);
        b = normalize(mul(float4(b, 0.0), worldViewInvTransposeMatrix).xyz);
        n = normalize(mul(float4(n, 0.0), worldViewInvTransposeMatrix).xyz);

        output.tangentToView0 = float3(t.x, b.x, n.x);
        output.tangentToView1 = float3(t.y, b.y, n.y);
        output.tangentToView2 = float3(t.z, b.z, n.z);
    #else
        output.normal = normalize(mul(float4(input.normal, 0.0), worldViewInvTransposeMatrix).xyz);
    #endif

    #if VERTEX_COLOR
        output.varVertexColor = half4(input.color0);
    #endif

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

    #if MATERIAL_DECAL || ALPHA_MASK
        output.varTexCoord1 = input.texcoord1.xy;
    #endif

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif

    output.projectedPosition = output.position;

    return output;
}
