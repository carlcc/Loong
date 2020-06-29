#shader vertex
#version 330 core

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
} vs_out;

void main()
{
    vs_out.Uv = v_Uv;
    vs_out.WorldNormal = normalize(mat3(transpose(inverse(ub_Model))) * v_Normal.xyz);
    vs_out.WorldPos = ub_Model * vec4(v_Pos, 1.0);
    vs_out.CameraPos = ub_ViewPos;
    gl_Position = ub_Projection * ub_View * vs_out.WorldPos;
}

#shader fragment
#version 330 core

#define MAX_LIGHT_COUNT 32
#define PI 3.141592653589793238

// material
uniform vec4        u_Albedo = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D   u_AlbedoMap;
uniform float       u_AlbedoMix = 1.0;

uniform float       u_Metallic = 0.0;
uniform sampler2D   u_MetallicMap;
uniform float       u_MetallicMix = 1.0;

uniform vec3        u_Reflectance = vec3(1.0, 1.0, 1.0);
uniform sampler2D   u_ReflectanceMap;
uniform float       u_ReflectanceMapMix = 1.0;

uniform float       u_Roughness = 1.0;
uniform sampler2D   u_RoughnessMap;
uniform float       u_RoughnessMix = 1.0;

uniform float       u_Emissive = 1.0;
uniform sampler2D   u_EmissiveMap;
uniform float       u_EmissiveMix = 1.0;

uniform float       u_Ao = 1.0;
uniform sampler2D   u_AoMap;
uniform float       u_AoMix = 1.0;

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

void prepareMaterialInput(out MaterialInput material, out vec2 uv)
{
    uv = u_TextureOffset + vec2(mod(fs_in.Uv.x * u_TextureTiling.x, 1), mod(fs_in.Uv.y * u_TextureTiling.y, 1));
    material.baseColor = mix(u_Albedo, texture(u_AlbedoMap, uv), u_AlbedoMix).rgb;
    material.metallic = mix(u_Metallic, texture(u_MetallicMap, uv).r, u_MetallicMix);
    material.reflectance = mix(u_Reflectance, texture(u_ReflectanceMap, uv).rgb, u_ReflectanceMapMix);
    material.roughness = mix(u_Roughness, texture(u_RoughnessMap, uv).r, u_RoughnessMix);
    material.clearCoat = u_ClearCoat;
    material.clearCoatRoughness = u_ClearCoatRoughness;
}

void main()
{
    vec3 worldPos = fs_in.WorldPos.xyz;
    vec3 n = normalize(fs_in.WorldNormal);
    vec3 v = normalize(fs_in.CameraPos - worldPos);

    MaterialInput material;
    vec2 uv;
    prepareMaterialInput(material, uv);
    // outColor = vec4(vec3(material.metallic), 1.0);
    // return;

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
    Lo *= cameraAperture;
    float gamma = 1/2.2;
    Lo = pow(Lo, vec3(gamma,gamma,gamma)); // gama correction
    outColor = vec4(Lo, 1.0);
}