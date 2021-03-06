#include "LoongResource/LoongRuntimeShader.h"

namespace Loong::Resource {

LoongRuntimeShaderCode LoongRuntimeShader::GenerateShaderSources() const
{
	LoongRuntimeShaderCode code {};
	code.vertexShader = R"(#version 330 core
)";
	if (IsUseAoMap()) { code.vertexShader += "#define USE_AO_MAP\n"; }
	if (IsUseMatallicMap()) { code.vertexShader += "#define USE_MATALLIC_MAP\n"; }
	if (IsUseEmissiveMap()) { code.vertexShader += "#define USE_EMISSIVE_MAP\n"; }
	if (IsUseEmissive()) { code.vertexShader += "#define USE_EMISSIVE\n"; }
	if (IsUseNormalMap()) { code.vertexShader += "#define USE_NORMAL_MAP\n"; }
	if (IsUseAlbedoMap()) { code.vertexShader += "#define USE_ALBEDO_MAP\n"; }
	if (IsUseRoughnessMap()) { code.vertexShader += "#define USE_ROUGHNESS_MAP\n"; }
	code.vertexShader += R"(
layout (location = 0) in vec3 v_Pos;
layout (location = 1) in vec2 v_Uv;
layout (location = 2) in vec3 v_Normal;
layout (location = 3) in vec3 v_Tan;
layout (location = 4) in vec3 v_BiTan;

layout (std140) uniform BasicUBO
{
    mat4    ub_Model;
    mat4    ub_View;
    mat4    ub_Projection;
    vec3    ub_ViewPos;
    float   ub_Time;
};

out VS_OUT
{
    vec2 Uv;
    vec3 WorldNormal;
    vec4 WorldPos;
    vec3 CameraPos;
    mat3 TBN;
} vs_out;

void main()
{
    vec3 T = normalize(vec3(ub_Model * vec4(v_Tan,   0.0)));
    vec3 B = normalize(vec3(ub_Model * vec4(v_BiTan, 0.0)));
    vec3 N = normalize(vec3(ub_Model * vec4(v_Normal,0.0)));

    vs_out.Uv = v_Uv;
    vs_out.TBN = mat3(T, B, N);
    vs_out.WorldNormal = normalize(mat3(transpose(inverse(ub_Model))) * v_Normal.xyz);
    vs_out.WorldPos = ub_Model * vec4(v_Pos, 1.0);
    vs_out.CameraPos = ub_ViewPos;
    gl_Position = ub_Projection * ub_View * vs_out.WorldPos;
}

)";

	code.fragmentShader = R"(#version 330 core
)";
	if (IsUseAoMap()) { code.fragmentShader += "#define USE_AO_MAP\n"; }
	if (IsUseMatallicMap()) { code.fragmentShader += "#define USE_MATALLIC_MAP\n"; }
	if (IsUseEmissiveMap()) { code.fragmentShader += "#define USE_EMISSIVE_MAP\n"; }
	if (IsUseEmissive()) { code.fragmentShader += "#define USE_EMISSIVE\n"; }
	if (IsUseNormalMap()) { code.fragmentShader += "#define USE_NORMAL_MAP\n"; }
	if (IsUseAlbedoMap()) { code.fragmentShader += "#define USE_ALBEDO_MAP\n"; }
	if (IsUseRoughnessMap()) { code.fragmentShader += "#define USE_ROUGHNESS_MAP\n"; }
	code.fragmentShader += R"(
#define MAX_LIGHT_COUNT 32
#define PI 3.141592653589793238

// material
#ifdef USE_ALBEDO_MAP
uniform sampler2D   u_Albedo;
#else
uniform vec4        u_Albedo = vec4(1.0, 1.0, 1.0, 1.0);
#endif

#ifdef USE_NORMAL_MAP
uniform sampler2D   u_Normal;
#endif

#ifdef USE_MATALLIC_MAP
uniform sampler2D   u_Metallic;
#else
uniform float       u_Metallic = 0.0;
#endif

uniform vec3        u_Reflectance = vec3(1.0, 1.0, 1.0);

#ifdef USE_ROUGHNESS_MAP
uniform sampler2D   u_Roughness;
#else
uniform float       u_Roughness = 1.0;
#endif

#ifdef USE_EMISSIVE
#ifdef USE_EMISSIVE_MAP
uniform sampler2D   u_Emissive;
#else
uniform vec3        u_Emissive = vec3(0.0);
#endif
uniform float       u_EmissiveFactor = 1.0;
#endif

#ifdef USE_AO_MAP
uniform sampler2D   u_Ao;
#else
uniform float       u_Ao = 1.0;
#endif

uniform float       u_ClearCoat = 1.0;
uniform float       u_ClearCoatRoughness = 1.0;

uniform vec2        u_TextureTiling = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset = vec2(0.0, 0.0);
// material end

uniform float       cameraAperture = 1.0;


out vec4 outColor;

in VS_OUT
{
    vec2 Uv;
    vec3 WorldNormal;
    vec4 WorldPos;
    vec3 CameraPos;
    mat3 TBN;
} fs_in;

// align to 4*sizeof(float)
struct Light {
    vec3    pos;
    float   lightType;

    vec3    dir;
    float   falloffRadius;

    vec3    color;
    float   intencity;

    float   innerAngle;
    float   outerAngle;
    // float   padding1_[2];
};

layout (std140) uniform LightUBO
{
    float   ub_lightsCount;
    //float   padding1_[3];// align to vec4
    Light   ub_lights[MAX_LIGHT_COUNT];
};

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

float NormalDistribution_GGX(float NoH, float a)
{
    return D_GGX(NoH, a);
}

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

float SmithGGXVisibilityCorrelated(float NoV, float NoL, float a)
{
    return V_SmithGGXCorrelated(NoV, NoL, a);
}

// F_Schlick(v,h,f0,f90) = f0 + (f90 - f0)(1 - v dot h)^5
// float f90 = saturate(dot(f0, vec3(50.0 * 0.33)));
// Where approximation f90 can be fixed 1.0

vec3 F_Schlick(float VoH, vec3 f0)
{
    vec3 f90 = vec3(1.0, 1.0, 1.0);
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float F_Schlick(float VoH, float f0)
{
    float f90 = 1.0;
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

vec3 F_Schlick(float VoH, vec3 f0, float f90)
{
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5);
}

float F_Schlick(float VoH, float f0, float f90)
{
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

vec3 F_SchlickRoughness(float VoH, vec3 F0, float roughness)
{
    vec3 r = vec3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness);
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
    vec3 baseColor;
    float metallic;
    vec3 reflectance;
    float roughness;
#ifdef USE_EMISSIVE
    vec3 emissive;
    float emissiveFactor;
#endif
    float clearCoat;
    float clearCoatRoughness;
    
};

vec3 BRDF(
    vec3 v,
    vec3 l,
    vec3 n,
    float NoL,
    float NoV,
    vec2 uv,
    MaterialInput material
) {
    vec3 h = normalize(v + l);

    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    float a = material.roughness * material.roughness;
    float metallic = material.metallic;
    vec3 baseColor = material.baseColor;
    vec3 diffuseColor = (1.0 - metallic) * baseColor;
    vec3 reflectance = material.reflectance; 
    vec3 f0 = mix(0.16 * reflectance * reflectance, baseColor, metallic);

    float D = D_GGX(NoH, a);
    float V = V_SmithGGXCorrelated(NoV, NoL, a);
    vec3  F = F_Schlick(NoH, f0);

    // specular BRDF
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    // can change to Fd_Lambert() to get better performance
    vec3 Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, a);//Fd_Lambert();

    // return Fr + Fd;
    // clear coat
    float clearCoatRoughness = clamp(material.clearCoatRoughness, 0.089, 1.0);
    float clearCoatA = clearCoatRoughness * clearCoatRoughness;
    float Dc = D_GGX(NoH, clearCoatA);
    float Vc = V_Kelemen(NoH);
    float Fc = F_Schlick(NoH, 0.04) * material.clearCoat;
    float Frc = Dc * Vc * Fc;
// 
    return (Fd + Fr) * (1.0 - Fc) + Frc;
    // apply lighting...
}

float getSquareFalloffAttenuation(vec3 posToLight, float falloffRadius) {
    float lightInvRadius = 1 / falloffRadius;
    float distanceSquare = dot(posToLight, posToLight);
    float factor = distanceSquare * lightInvRadius * lightInvRadius;
    float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float getSpotAngleAttenuation(vec3 l, vec3 lightDir, float innerAngle, float outerAngle)
{
    // the scale and offset computations can be done CPU-side
    float cosOuter = cos(outerAngle);
    float spotScale = 1.0 / max(cos(innerAngle) - cosOuter, 1e-4);
    float spotOffset = -cosOuter * spotScale;
    float cd = dot(normalize(-lightDir), l);
    float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
    return attenuation * attenuation;
}

vec3 ComputeLight(in vec3 l, in vec3 n, in vec3 v, in Light light, vec2 uv, in MaterialInput material)
{
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoV = abs(dot(n, v)) + 1e-5;
    if (NoL > 0 && NoV > 0) {
        vec3 brdf = BRDF(v, l, n, NoL, NoV, uv, material);
        return light.color * light.intencity * brdf * NoL;
    } else {
        return vec3(0.0, 0.0, 0.0);
    }
}

vec3 SRGB2Linear(vec3 i)
{
    return pow(i, vec3(2.2, 2.2, 2.2));
}

vec3 Linear2SRGB(vec3 i)
{
    float gamma = 1/2.2;
    return pow(i, vec3(gamma,gamma,gamma));
}

void prepareMaterialInput(out MaterialInput material, out vec2 uv)
{
    uv = u_TextureOffset + vec2(mod(fs_in.Uv.x * u_TextureTiling.x, 1), mod(fs_in.Uv.y * u_TextureTiling.y, 1));

#ifdef USE_ALBEDO_MAP
    material.baseColor = SRGB2Linear(texture(u_Albedo, uv).rgb);
#else
    material.baseColor = u_Albedo.rgb;
#endif

#ifdef USE_MATALLIC_MAP
    material.metallic = texture(u_Metallic, uv).r;
#else
    material.metallic = u_Metallic;
#endif

    material.reflectance = u_Reflectance;

#ifdef USE_ROUGHNESS_MAP
    material.roughness = texture(u_Roughness, uv).r;
#else
    material.roughness = u_Roughness;
#endif
    material.clearCoat = u_ClearCoat;
    material.clearCoatRoughness = u_ClearCoatRoughness;

#ifdef USE_EMISSIVE
#ifdef USE_EMISSIVE_MAP
    material.emissive = SRGB2Linear(texture(u_Emissive, uv).rgb);
#else
    material.emissive = SRGB2Linear(u_Emissive);
#endif
    material.emissiveFactor = u_EmissiveFactor;
#endif
}

void main()
{
    MaterialInput material;
    vec2 uv;
    prepareMaterialInput(material, uv);

    vec3 worldPos = fs_in.WorldPos.xyz;
    vec3 v = normalize(fs_in.CameraPos - worldPos);

#ifdef USE_NORMAL_MAP
    vec3 n = texture(u_Normal, uv).xyz * 2.0 - 1.0;
    n = normalize(fs_in.TBN * n);
#else
    vec3 n = fs_in.WorldNormal;
#endif

    vec3 Lo = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < ub_lightsCount; ++i) {
        Light light = ub_lights[i];
        if (light.lightType == 0) {
            // directional
            vec3 l = normalize(light.dir);
            Lo += ComputeLight(l, n, v, light, uv, material);
        } else if (light.lightType == 1) {
            // point
            vec3 posToLight = light.pos - worldPos;
            vec3 l = normalize(posToLight);
            vec3 contrib = ComputeLight(l, n, v, light, uv, material);
            float attenuation = getSquareFalloffAttenuation(posToLight, light.falloffRadius);
            Lo += contrib * attenuation;
        } else {
            // spot
            vec3 posToLight = light.pos - worldPos;
            vec3 lightDir = normalize(light.dir);
            vec3 l = normalize(posToLight);
            vec3 contrib = ComputeLight(l, n, v, light, uv, material);
            float attenuation = getSquareFalloffAttenuation(posToLight, light.falloffRadius);
            attenuation *= getSpotAngleAttenuation(l, lightDir, light.innerAngle, light.outerAngle);
            Lo += contrib * attenuation;
        }
    }
#ifdef USE_EMISSIVE
    Lo += material.emissive * material.emissiveFactor;
#endif
    Lo *= cameraAperture;
    Lo = Linear2SRGB(Lo); // gama correction
    outColor = vec4(Lo, 1.0);
})";

	return code;
}

}
