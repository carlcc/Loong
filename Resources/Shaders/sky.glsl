#shader vertex
#version 330 core

layout (location = 0) in vec3 v_Pos;

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
    vec3 TexPos;
} vs_out;

void main()
{
    vs_out.TexPos = v_Pos;
    vec4 tmp = ub_Projection * mat4(mat3(ub_View)) * vec4(v_Pos, 1.0);
    gl_Position = tmp.xyww;
}

#shader fragment
#version 330 core

#define PI 3.141592653589793238

out vec4 outColor;

in VS_OUT
{
    vec3 TexPos;
} fs_in;

uniform sampler2D   u_Panorama;

void main()
{   
    vec3 dir = normalize(fs_in.TexPos);
	vec2 uv = vec2(atan(dir.z, dir.x), acos(dir.y));
	uv /= vec2( 2 * PI, -PI);
    uv.x -= 0.25;
    outColor = textureLod(u_Panorama, uv, 0);
}