
#define PI 3.141592653589793238

struct PSInput
{
    float4 Pos          : SV_POSITION;
    float2 Uv0          : TEX_COORD0;
    float2 Uv1          : TEX_COORD1;
    float3 WorldNormal  : NORMAL;
    float4 WorldPos     : POSITIONT;
    float3 CameraPos    : LG_EYEPOS;
    float3x3 TBN        : LG_TBN;
};

#ifdef IS_VERTEX_SHADER

cbuffer Constants
{
    float4x4 ub_MVP;
    float4x4 ub_Model;
    float4x4 ub_View;
    float4x4 ub_Projection;
    float3   ub_ViewPos;
    float    ub_Time;
};

struct VSInput
{
    float3 v_Pos    : ATTRIB0;
    float2 v_Uv0    : ATTRIB1;
    float2 v_Uv1    : ATTRIB2;
    float3 v_Normal : ATTRIB3;
    float3 v_Tan    : ATTRIB4;
    float3 v_BiTan  : ATTRIB5;
};

void main(in  VSInput VSIn,
          out PSInput PSIn)
{
    float4 pos = float4(VSIn.v_Pos,1.0);

    PSIn.Pos = mul(pos, ub_MVP);
    PSIn.Uv0  = float2(VSIn.v_Uv0.x, 1 - VSIn.v_Uv0.y);
    PSIn.Uv1  = float2(VSIn.v_Uv1.x, 1 - VSIn.v_Uv1.y);
    PSIn.WorldPos = mul(pos, ub_Model);
    PSIn.CameraPos = ub_ViewPos;

    float3 T = normalize(mul(float4(VSIn.v_Tan, 0.0), ub_Model).xyz);
    float3 B = normalize(mul(float4(VSIn.v_BiTan, 0.0), ub_Model).xyz);
    float3 N = normalize(mul(float4(VSIn.v_Normal, 0.0), ub_Model).xyz);
    PSIn.TBN = float3x3(T, B, N);
}

#endif

#ifdef IS_PIXEL_SHADER

#define USE_ALBEDO_MAP
#define USE_NORMAL_MAP
#define USE_MATALLIC_MAP
#define USE_ROUGHNESS_MAP
#define USE_EMISSIVE
#define USE_EMISSIVE_MAP

Texture2D    g_Albedo;
SamplerState g_Albedo_sampler;
Texture2D    g_Normal;
SamplerState g_Normal_sampler;
Texture2D    g_Roughness;
SamplerState g_Roughness_sampler;
Texture2D    g_Metallic;
SamplerState g_Metallic_sampler;
#ifdef USE_EMISSIVE
Texture2D    g_Emissive;
SamplerState g_Emissive_sampler;
#endif

#define MAX_LIGHT_COUNT 32

// a = remapped roughness
//                               a*a
// D_GGX(h, a) = ------------------------------------
//                PI * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1;
    return a2 / (PI * f * f);

    // This is another implenent, seems more efficient
    // float a2 = NoH * a;
    // float k = a2 / (1.0 - NoH * NoH + a2 * a2);
    // return k * k * (1.0 / PI);
}

float NormalDistribution_GGX(float NoH, float a) { return D_GGX(NoH, a); }

//                                                                 0.5
// V_SmithGGXCorrelated(v, l, a) = ------------------------------------------------------------------------
//                                  nol * sqrt(nov^2 * (1 - a^2) + a^2) + nov * sqrt(nol^2 * (1-a^2) + a^2)
float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);

    // We can optimize this visibility function by using an approximation after noticing that all the terms 
    // under the square roots are squares and that all the terms are in the [0..1] range:
    //                                                              0.5
    // V_SmithGGXCorrelated(v, l, a) = ----------------------------------------------------
    //                                  nol * (nov * (1 - a) + a) + nov * (nol * (1-a) + a)
    // float GGXV = NoL * (NoV * (1.0 - a) + a);
    // float GGXL = NoV * (NoL * (1.0 - a) + a);
    // return 0.5 / (GGXV + GGXL);
}

float SmithGGXVisibilityCorrelated(float NoV, float NoL, float a) { return V_SmithGGXCorrelated(NoV, NoL, a); }

// F_Schlick(v,h,f0,f90) = f0 + (f90 - f0)(1 - v dot h)^5
// float f90 = saturate(dot(f0, float3(50.0 * 0.33)));
// Where approximation f90 can be fixed 1.0

float3 F_Schlick(float VoH, float3 f0)
{
    float3 f90 = float3(1.0, 1.0, 1.0);
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float F_Schlick(float VoH, float f0)
{
    float f90 = 1.0;
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float3 F_Schlick(float VoH, float3 f0, float f90)
{
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5);
}

float F_Schlick(float VoH, float f0, float f90)
{
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float3 F_SchlickRoughness(float VoH, float3 F0, float roughness)
{
    float3 r = float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness);
    return F0 + (max(r, F0) - F0) * pow(1.0 - VoH, 5.0);
}   

float Fd_Lambert()
{
    return 1.0 / PI;
}

float Fd_Burley(float NoV, float NoL, float LoH, float a)
{
    float f90 = 0.5 + 2.0 * a * LoH * LoH;
    float lightScatter = F_Schlick(NoL, 1.0, f90);
    float viewScatter = F_Schlick(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}

float V_Kelemen(float LoH)
{
    return 0.25 / (LoH * LoH);
}

struct MaterialInput {
    float3 baseColor;
    float metallic;
    float3 reflectance;
    float roughness;
#ifdef USE_EMISSIVE
    float3 emissive;
    float emissiveFactor;
#endif
    float clearCoat;
    float clearCoatRoughness;
    
};

// align to 4*sizeof(float)
struct Light {
    float3    pos;
    float     lightType;

    float3    dir;
    float     falloffRadius;

    float3    color;
    float     intencity;

    float     innerAngle;
    float     outerAngle;
    float2    padding1_;
};

cbuffer PSLightUniforms {
    float   ub_LightsCount;
    float3  padding1_;// align to vec4
    Light   ub_Lights[MAX_LIGHT_COUNT];
};

float3 BRDF(
    float3 v,
    float3 l,
    float3 n,
    float NoL,
    float NoV,
    float2 uv,
    MaterialInput material
) {
    float3 h = normalize(v + l);

    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    float a = material.roughness * material.roughness;
    float metallic = material.metallic;
    float3 baseColor = material.baseColor;
    float3 diffuseColor = (1.0 - metallic) * baseColor;
    float3 reflectance = material.reflectance; 
    float3 f0 = lerp(0.16 * reflectance * reflectance, baseColor, metallic);

    float D = D_GGX(NoH, a);
    float V = V_SmithGGXCorrelated(NoV, NoL, a);
    float3  F = F_Schlick(NoH, f0);

    // specular BRDF
    float3 Fr = (D * V) * F;

    // diffuse BRDF
    // can change to Fd_Lambert() to get better performance
    float3 Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, a);//Fd_Lambert();

    // return Fr + Fd;
    // clear coat
    float clearCoatRoughness = clamp(material.clearCoatRoughness, 0.089, 1.0);
    float clearCoatA = clearCoatRoughness * clearCoatRoughness;
    float Dc = D_GGX(NoH, clearCoatA);
    float Vc = V_Kelemen(NoH);
    float Fc = F_Schlick(NoH, 0.04) * material.clearCoat;
    float Frc = Dc * Vc * Fc;

    return (Fd + Fr) * (1.0 - Fc) + Frc;
    // apply lighting...
}

float getSquareFalloffAttenuation(float3 posToLight, float falloffRadius) {
    float lightInvRadius = 1 / falloffRadius;
    float distanceSquare = dot(posToLight, posToLight);
    float factor = distanceSquare * lightInvRadius * lightInvRadius;
    float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float getSpotAngleAttenuation(float3 l, float3 lightDir, float innerAngle, float outerAngle)
{
    // the scale and offset computations can be done CPU-side
    float cosOuter = cos(outerAngle);
    float spotScale = 1.0 / max(cos(innerAngle) - cosOuter, 1e-4);
    float spotOffset = -cosOuter * spotScale;
    float cd = dot(normalize(-lightDir), l);
    float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
    return attenuation * attenuation;
}

float3 ComputeLight(in float3 l, in float3 n, in float3 v, in Light light, float2 uv, in MaterialInput material)
{
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoV = abs(dot(n, v)) + 1e-5;
    if (NoL > 0 && NoV > 0) {
        float3 brdf = BRDF(v, l, n, NoL, NoV, uv, material);
        return light.color * light.intencity * brdf * NoL;
    } else {
        return float3(0.0, 0.0, 0.0);
    }
}

float3 SRGB2Linear(float3 i)
{
    return pow(i, float3(2.2, 2.2, 2.2));
}

float3 Linear2SRGB(float3 i)
{
    float gamma = 1/2.2;
    return pow(i, float3(gamma,gamma,gamma));
}

cbuffer PSMaterialUniforms {
    float2 ub_TextureOffset;
    float2 ub_TextureTiling;
    float3 ub_Albedo;
    float  ub_Metallic;
    float3 ub_Reflectance;
    float  ub_Roughness;
    float3 ub_Emissive;
    float  ub_EmissiveFactor;
    float  ub_ClearCoat;
    float  ub_ClearCoatRoughness;
};

void prepareMaterialInput(PSInput psIn, out MaterialInput material, out float2 uv)
{
    uv = ub_TextureOffset + float2(fmod(psIn.Uv1.x * ub_TextureTiling.x, 1), fmod(psIn.Uv1.y * ub_TextureTiling.y, 1));

#ifdef USE_ALBEDO_MAP
    material.baseColor = (g_Albedo.Sample(g_Albedo_sampler, uv).rgb);
#else
    material.baseColor = (ub_Albedo.rgb);
#endif

#ifdef USE_MATALLIC_MAP
    material.metallic = g_Metallic.Sample(g_Metallic_sampler, uv).r;
#else
    material.metallic = ub_Metallic;
#endif

    material.reflectance = ub_Reflectance;

#ifdef USE_ROUGHNESS_MAP
    material.roughness = g_Roughness.Sample(g_Roughness_sampler, uv).r;
#else
    material.roughness = ub_Roughness;
#endif
    material.clearCoat = ub_ClearCoat;
    material.clearCoatRoughness = ub_ClearCoatRoughness;

#ifdef USE_EMISSIVE
#ifdef USE_EMISSIVE_MAP
    material.emissive = (g_Emissive.Sample(g_Emissive_sampler, uv).rgb);
#else
    material.emissive = (ub_Emissive);
#endif
    material.emissiveFactor = ub_EmissiveFactor;
#endif
}

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    MaterialInput material;
    float2 uv;
    prepareMaterialInput(PSIn, material, uv);

    float3 worldPos = PSIn.WorldPos.xyz;
    float3 v = normalize(PSIn.CameraPos - worldPos);


#ifdef USE_NORMAL_MAP
    float3 n = Linear2SRGB(g_Normal.Sample(g_Normal_sampler, uv).xyz) * 2.0 - 1.0;
    n = normalize(mul(n, PSIn.TBN));
#else
    float3 n = PSIn.WorldNormal;
#endif

    float3 Lo = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < ub_LightsCount; ++i) {
        Light light = ub_Lights[i];
        if (light.lightType == 0) {
            // directional
            float3 l = normalize(light.dir);
            Lo += ComputeLight(l, n, v, light, uv, material);
        } else if (light.lightType == 1) {
            // point
            float3 posToLight = light.pos - worldPos;
            float3 l = normalize(posToLight);
            float3 contrib = ComputeLight(l, n, v, light, uv, material);
            float attenuation = getSquareFalloffAttenuation(posToLight, light.falloffRadius);
            Lo += contrib * attenuation;
        } else {
            // spot
            float3 posToLight = light.pos - worldPos;
            float3 lightDir = normalize(light.dir);
            float3 l = normalize(posToLight);
            float3 contrib = ComputeLight(l, n, v, light, uv, material);
            float attenuation = getSquareFalloffAttenuation(posToLight, light.falloffRadius);
            attenuation *= getSpotAngleAttenuation(l, lightDir, light.innerAngle, light.outerAngle);
            Lo += contrib * attenuation;
        }
    }
#ifdef USE_EMISSIVE
    Lo += material.emissive * material.emissiveFactor;
#endif
    Lo = (Lo); // gama correction
    PSOut.Color = float4(Lo, 1.0);
}

#endif