#shader vertex
#version 330 core

layout (location = 0) in vec3 v_Pos;
layout (location = 1) in vec2 v_Uv;
layout (location = 2) in vec3 v_Normal;

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

out vec4 outColor;

in VS_OUT
{
    vec2 Uv;
} fs_in;

uniform vec4        u_Diffuse = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D   u_DiffuseMap;
uniform vec2        u_TextureTiling = vec2(1.0, 1.0);
uniform vec2        u_TextureOffset = vec2(0.0, 0.0);

void main()
{
    outColor = texture(u_DiffuseMap, u_TextureOffset + vec2(mod(fs_in.Uv.x * u_TextureTiling.x, 1), mod(fs_in.Uv.y * u_TextureTiling.y, 1))) * u_Diffuse;
}