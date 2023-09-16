#version 460 core

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_pos;
layout (location = 3) in vec4 in_color;
layout (location = 4) in mat4 in_instance_model_matrix;

out vec2 vs_uv;
out vec3 vs_normal;
out vec3 vs_pos;
out vec4 vs_color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_pos = vec3(in_instance_model_matrix * vec4(in_pos, 1.0));
    vs_normal = mat3(transpose(inverse(in_instance_model_matrix))) * in_normal;
    vs_uv = in_uv;
    vs_color = in_color;

    gl_Position = projection * view * vec4(vs_pos, 1.0);
}
