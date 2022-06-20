#include "common.slh"

vertex_in
{
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
    float4 color0 : COLOR0;

    float4 pivot : TEXCOORD4;
    
    #if WIND_ANIMATION
        float flexibility : TEXCOORD5;
        float2 angleSinCos : TEXCOORD6;
    #endif
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;
    [lowp] half4 varVertexColor : COLOR1;

    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif
    
    #if RECEIVE_SHADOW
        float4 worldPos : COLOR2;
        float4 projPos : COLOR3;
    #endif
};

////////////////////////////////////////////////////////////////////////////////
// properties

[auto][a] property float4x4 worldViewProjMatrix;

[auto][a] property float3 worldScale;
[auto][a] property float4x4 projMatrix;
[auto][a] property float4x4 worldViewMatrix;

#if CUT_LEAF
    [material][a] property float cutDistance = 1.0;
#endif

#if !SPHERICAL_LIT  //legacy for old tree lighting
    [material][a] property float4 treeLeafColorMul = float4(0.5,0.5,0.5,0.5) ;
    [material][a] property float treeLeafOcclusionOffset = 0.0 ;
    [material][a] property float treeLeafOcclusionMul = 0.5 ;
#endif

#if WIND_ANIMATION
    [auto][a] property float2 leafOscillationParams; //x: A*sin(T); y: A*cos(T);
#endif

#if SPHERICAL_LIT
    [auto][a] property float speedTreeLightSmoothing;
#endif

#if VERTEX_DISTORTION && VERTEX_SHADER_TEXTURE_FETCH_SUPPORTED
    uniform sampler2D vertexDistortionNoise; // Put a different perlin noise into each of rgb channels.

    [auto][a] property float4x4 viewProjMatrix;

    [material][a] property float vertexDistortionScale = 0.0f;
    [material][a] property float2 vertexDistortionVariationSpeed;
    [material][a] property float3 vertexDistortionWindVector;
#endif

#if VERTEX_FOG && FOG_ATMOSPHERE
    [auto][a] property float4 lightPosition0;
#endif

#include "vp-fog-props.slh"

#if WIND_ANIMATION
    [auto][a] property float2 trunkOscillationParams;
#endif

#if SPHERICAL_LIT
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
#endif

#if TEXTURE0_SHIFT_ENABLED
    [material][a] property float2 texture0Shift = float2(0, 0);
#endif

#if TEXTURE0_ANIMATION_SHIFT
    [material][a] property float2 tex0ShiftPerSecond = float2(0, 0);
#endif

#if VERTEX_FOG || VERTEX_DISTORTION
    [auto][a] property float3 cameraPosition;
#endif

#if VERTEX_FOG || VERTEX_DISTORTION || RECEIVE_SHADOW
  [auto][a] property float4x4 worldMatrix;
#endif

#if WAVE_ANIMATION || TEXTURE0_ANIMATION_SHIFT || VERTEX_DISTORTION
    [auto][a] property float globalTime;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out  output;

    float3 position = lerp(input.position.xyz, input.pivot.xyz, input.pivot.w);
    float3 billboardOffset = input.position.xyz - position.xyz;

    #if CUT_LEAF
        float pivotDistance = dot(position.xyz, float3(worldViewMatrix[0].z, worldViewMatrix[1].z, worldViewMatrix[2].z)) + worldViewMatrix[3].z;
        billboardOffset *= step(-cutDistance, pivotDistance);
    #endif

    #if WIND_ANIMATION
        //inAngleSinCos:          x: cos(T0);  y: sin(T0);
        //leafOscillationParams:  x: A*sin(T); y: A*cos(T);
        float3 windVectorFlex = float3(trunkOscillationParams * input.flexibility, 0.0);
        position += windVectorFlex;
        
        float2 SinCos = input.angleSinCos * leafOscillationParams; //vec2(A*sin(t)*cos(t0), A*cos(t)*sin(t0))
        float sinT = SinCos.x + SinCos.y;     //sin(t+t0)*A = sin*cos + cos*sin
        float cosT = 1.0 - 0.5 * sinT * sinT; //cos(t+t0)*A = 1 - 0.5*sin^2
        
        float4 SinCosT = float4(sinT, cosT, cosT, sinT); //temp vec for mul
        float4 offsetXY = float4(billboardOffset.x, billboardOffset.y, billboardOffset.x, billboardOffset.y); //temp vec for mul
        float4 rotatedOffsetXY = offsetXY * SinCosT; //vec4(x*sin, y*cos, x*cos, y*sin)
        
        billboardOffset.x = rotatedOffsetXY.z - rotatedOffsetXY.w; //x*cos - y*sin
        billboardOffset.y = rotatedOffsetXY.x + rotatedOffsetXY.y; //x*sin + y*cos
    #endif
    
    float4 eyeCoordsPosition4 = mul(float4(position, 1.0), worldViewMatrix) + float4(worldScale * billboardOffset, 0.0);

    output.position = mul(eyeCoordsPosition4, projMatrix);

    #if VERTEX_FOG
        #if FOG_HALFSPACE || FOG_ATMOSPHERE_MAP
            float3 FOG_world_position  = mul( float4(input.position.xyz,1.0), worldMatrix ).xyz;
        #endif
        #define FOG_eye_position cameraPosition
        #define FOG_view_position eyeCoordsPosition4.xyz
        #define FOG_in_position input.position
        #define FOG_to_light_dir lightPosition0.xyz
        #include "vp-fog-math.slh"
        output.varFog = half4(FOG_result);
    #endif

    output.varVertexColor = half4(input.color0);

    #if SPHERICAL_LIT
        #if SPHERICAL_HARMONICS_4 || SPHERICAL_HARMONICS_9
            float3 sphericalLightFactor = 0.282094 * sphericalHarmonics[0].xyz;
        #else
            float3 sphericalLightFactor = 0.282094 * sphericalHarmonics.xyz;
        #endif
        
        float3 localSphericalLightFactor = sphericalLightFactor;
        
        #if !CUT_LEAF
            #if SPHERICAL_HARMONICS_4 || SPHERICAL_HARMONICS_9
                float3x3 invViewMatrix3 = float3x3(float3(invViewMatrix[0].xyz), float3(invViewMatrix[1].xyz), float3(invViewMatrix[2].xyz));
                float3 normal = mul((eyeCoordsPosition4.xyz - worldViewObjectCenter), invViewMatrix3);
                normal /= boundingBoxSize;
                float3 n = normalize(normal);

                float3x3 shMatrix = float3x3(float3(sphericalHarmonics[0].w,  sphericalHarmonics[1].xy),
                                            float3(sphericalHarmonics[1].zw, sphericalHarmonics[2].x),
                                            float3(sphericalHarmonics[2].yzw));
                sphericalLightFactor += 0.325734 * mul(float3(n.y, n.z, n.x), shMatrix);

                float3 localNormal = mul((worldScale * billboardOffset), invViewMatrix3);
                localNormal.z += 1.0 - input.pivot.w; //in case regular geometry (not billboard) we have zero 'localNoraml', so add something to correct 'normalize'
                float3 ln = normalize(localNormal);
                localSphericalLightFactor += (0.325734 * mul(float3(ln.y, ln.z, ln.x), shMatrix)) * input.pivot.w;

                #if SPHERICAL_HARMONICS_9
                    sphericalLightFactor += (0.273136 * (n.y * n.x)) * float3(sphericalHarmonics[3].xyz);
                    sphericalLightFactor += (0.273136 * (n.y * n.z)) * float3(sphericalHarmonics[3].w,  sphericalHarmonics[4].xy);
                    sphericalLightFactor += (0.078847 * (3.0 * n.z * n.z - 1.0)) * float3(sphericalHarmonics[4].zw, sphericalHarmonics[5].x);
                    sphericalLightFactor += (0.273136 * (n.z * n.x))  * float3(sphericalHarmonics[5].yzw);
                    sphericalLightFactor += (0.136568 * (n.x * n.x - n.y * n.y)) * float3(sphericalHarmonics[6].xyz);
                #endif

                sphericalLightFactor = lerp(sphericalLightFactor, localSphericalLightFactor, speedTreeLightSmoothing);
            #endif // SPHERICAL_HARMONICS_4 || SPHERICAL_HARMONICS_9
        #endif // !CUT_LEAF

        output.varVertexColor.xyz = half3(input.color0.xyz) * half3(sphericalLightFactor * 2.0);
        output.varVertexColor.w = half(1.0);
    #else //legacy for old tree lighting
        output.varVertexColor.xyz = half3(input.color0.xyz * treeLeafColorMul.xyz * treeLeafOcclusionMul + float3(treeLeafOcclusionOffset,treeLeafOcclusionOffset,treeLeafOcclusionOffset));
    #endif

    output.varTexCoord0.xy = input.texcoord0;

    #if TEXTURE0_SHIFT_ENABLED
        output.varTexCoord0.xy += texture0Shift;
    #endif

    #if TEXTURE0_ANIMATION_SHIFT
        output.varTexCoord0.xy += frac(tex0ShiftPerSecond * globalTime);
    #endif

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif
    
    #if RECEIVE_SHADOW
        output.worldPos = mul(float4(position, 1.0), worldMatrix);
        output.projPos = output.position;
    #endif

    return output;
}
