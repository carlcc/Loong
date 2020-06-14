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

void main()
{
    gl_Position = ub_Projection * ub_View * ub_Model * vec4(v_Pos, 1.0);
}

#shader fragment
#version 330 core

out vec4 outColor;

uniform vec4        u_wireColor;

void main()
{
    outColor = u_wireColor;
}