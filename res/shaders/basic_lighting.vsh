#version 460 core

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_pos;

out vec2 vs_uv;
out vec3 vs_normal;
out vec3 vs_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_pos = vec3(model * vec4(in_pos, 1.0));
    vs_normal = mat3(transpose(inverse(model))) * in_normal;
    vs_uv = in_uv;

    gl_Position = projection * view * vec4(vs_pos, 1.0);
}