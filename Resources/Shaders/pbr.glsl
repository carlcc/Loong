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
} vs_out;

void main()
{
    vs_out.Uv = v_Uv;

    gl_Position = ub_Projection * ub_View * ub_Model * vec4(v_Pos, 1.0);
}

    #shader fragment
    #version 330 core

    #define MAX_LIGHT_COUNT 32
    #define PI 3.141592653589793238

out vec4 outColor;

in VS_OUT
{
    vec2 Uv;
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
    float   padding1_[2];
};

layout (std140) uniform LightUBO
{
    float   ub_lightsCount;
//float   padding1_[3];// align to vec4
    Light   ub_lights[MAX_LIGHT_COUNT];
};

uniform vec4        u_Diffuse = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D   u_DiffuseMap;
uniform sampler2D   u_SpecularMap;
uniform vec2        u_TextureTiling = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset = vec2(0.0, 0.0);

void main()
{
    if (ub_lightsCount == 1.0) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (ub_lightsCount == 2) {
        outColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    //    vec4 tmpOutColor = vec4(ub_lightsCount * 0.25);
    //    tmpOutColor.a = 0.5;
    //    for (int i = 0; i < ub_lightsCount; ++i) {
    //        tmpOutColor.xyz = 0.5 * texture(u_DiffuseMap, u_TextureOffset + vec2(mod(fs_in.Uv.x * u_TextureTiling.x, 1), mod(fs_in.Uv.y * u_TextureTiling.y, 1))).xyz * u_Diffuse.xyz;
    //        tmpOutColor.xyz += 0.5 *  texture(u_SpecularMap, u_TextureOffset + vec2(mod(fs_in.Uv.x * u_TextureTiling.x, 1), mod(fs_in.Uv.y * u_TextureTiling.y, 1))).xyz * u_Diffuse.xyz;
    //    }
    //    outColor = tmpOutColor;
}