#include "common.slh"

vertex_in
{    
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
    float4 color0 : COLOR0;

    #if FRAME_BLEND || PARTICLES_FLOWMAP_ANIMATION
        float3 texcoord1 : TEXCOORD1; // uv1.xy + time
    #endif

    #if PARTICLES_FLOWMAP || PARTICLES_FLOWMAP_ANIMATION
        float4 texcoord2 : TEXCOORD2; // Flow speed and flow offset.
    #endif

    #if PARTICLES_NOISE
        float3 texcoord3 : TEXCOORD3; // Noise uv and scale.
    #endif

    #if PARTICLES_FRESNEL_TO_ALPHA || PARTICLES_ALPHA_REMAP || PARTICLES_PERSPECTIVE_MAPPING
        float3 texcoord5 : TEXCOORD5;  // x - fresnel. y - alpha remap. z - perspective mapping w.
    #endif

    #if PARTICLES_FLOWMAP_ANIMATION
        float2 flowmapCrossfadeData : TEXCOORD7;
    #endif
};

vertex_out
{
    float4 position : SV_POSITION;
    [lowp] half4 varVertexColor : COLOR1;

    #if PARTICLES_PERSPECTIVE_MAPPING
        float3 varTexCoord0 : TEXCOORD0;
    #else
        float2 varTexCoord0 : TEXCOORD0;
    #endif

    #if FRAME_BLEND || PARTICLES_FLOWMAP_ANIMATION
        float2 varTexCoord1 : TEXCOORD1;
    #endif

    #if PARTICLES_FLOWMAP || PARTICLES_FLOWMAP_ANIMATION
        float2 varParticleFlowTexCoord : TEXCOORD2;
    #endif
    
    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD5;
    #endif

    #if PARTICLES_NOISE
        #if PARTICLES_FRESNEL_TO_ALPHA
            float4 varTexcoord6 : TEXCOORD6; // Noise uv and scale. Fresnel a.
        #else
            float3 varTexcoord6 : TEXCOORD6; // Noise uv and scale.
        #endif
    #elif PARTICLES_FRESNEL_TO_ALPHA
        float varTexcoord6 : TEXCOORD6; // Fresnel a.
    #endif

    #if FRAME_BLEND && PARTICLES_ALPHA_REMAP
        half2 varTexcoord3 : TEXCOORD3;
    #elif FRAME_BLEND || PARTICLES_ALPHA_REMAP || PARTICLES_FLOWMAP_ANIMATION
        half varTexcoord3 : TEXCOORD3;
    #endif

    #if PARTICLES_FLOWMAP || PARTICLES_FLOWMAP_ANIMATION
        [lowp] float3 varFlowData : TEXCOORD4; // For flowmap animations - xy next frame uv. z - frame time
    #endif

    #if SOFT_PARTICLES
        float4 projectedPosition : TEXCOORD7;
    #endif
};

[auto][a] property float4x4 worldViewProjMatrix;

#if VERTEX_FOG && FOG_ATMOSPHERE
    [auto][a] property float4 lightPosition0;
#endif

#if VERTEX_FOG
    [auto][a] property float4x4 worldViewMatrix;
#endif

#include "vp-fog-props.slh"

#if VERTEX_FOG
    [auto][a] property float3 cameraPosition;
    [auto][a] property float4x4 worldMatrix;
#endif

#if PARTICLES_FLOWMAP
    [auto][a] property float globalTime;
#endif

vertex_out vp_main(vertex_in input)
{
    vertex_out  output;

    #if PARTICLES_FLOWMAP && !PARTICLES_FLOWMAP_ANIMATION
        float flowSpeed = input.texcoord2.z;
        float flowOffset = input.texcoord2.w;
        output.varParticleFlowTexCoord = input.texcoord2.xy;

        float scaledTime = globalTime * flowSpeed;
        float2 flowPhases = frac(float2(scaledTime, scaledTime+0.5))-float2(0.5, 0.5);
        float flowBlend = abs(flowPhases.x*2.0);
        output.varFlowData = float3(flowPhases * flowOffset, flowBlend);
    #elif PARTICLES_FLOWMAP_ANIMATION
        float flowOffset = input.texcoord2.w;
        output.varParticleFlowTexCoord = input.texcoord2.xy;
        output.varFlowData.xy = input.flowmapCrossfadeData.xy;
        output.varFlowData.z = flowOffset;
    #endif

    #if PARTICLES_NOISE
        output.varTexcoord6.xyz = input.texcoord3.xyz;
    #endif

    float4 modelPos = float4(input.position.xyz, 1.0);
    output.position = mul(modelPos, worldViewProjMatrix);

    #if SOFT_PARTICLES
        output.projectedPosition = output.position;
    #endif

    #if PARTICLES_FRESNEL_TO_ALPHA
        // intel metal shader compiler strips data as unused without "* 1.0042"
        #if PARTICLES_NOISE
            output.varTexcoord6.w = input.texcoord5.x * 1.0042;
        #else
            output.varTexcoord6 = input.texcoord5.x * 1.0042;
        #endif 
    #endif

    #if VERTEX_FOG
        float3 FOG_view_position = mul(modelPos, worldViewMatrix).xyz;
        #if FOG_HALFSPACE || FOG_ATMOSPHERE_MAP
            float3 FOG_world_position = mul(modelPos, worldMatrix).xyz;
        #endif
        #define FOG_eye_position cameraPosition
        #define FOG_in_position input.position
        #define FOG_to_light_dir lightPosition0.xyz
        #include "vp-fog-math.slh"
        output.varFog = half4(FOG_result);
    #endif

    output.varVertexColor = half4(input.color0);

    output.varTexCoord0.xy = input.texcoord0;
    #if PARTICLES_PERSPECTIVE_MAPPING
        output.varTexCoord0.z = input.texcoord5.z;
    #endif

    #if FRAME_BLEND || PARTICLES_FLOWMAP_ANIMATION
        output.varTexCoord1 = input.texcoord1.xy;
    #endif

    #if FRAME_BLEND || PARTICLES_FLOWMAP_ANIMATION
        #if PARTICLES_ALPHA_REMAP
            output.varTexcoord3.x = input.texcoord1.z;
            output.varTexcoord3.y = input.texcoord5.y;
        #else
            output.varTexcoord3 = input.texcoord1.z;
        #endif
    #elif PARTICLES_ALPHA_REMAP
        output.varTexcoord3 = input.texcoord5.y;
    #endif

    #if FORCE_2D_MODE
        output.position.z = 0.0;
    #endif

    return output;
}
