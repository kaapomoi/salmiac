#version 460 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in vec2 vs_uv;
in vec3 vs_normal;
in vec3 vs_pos;
in vec4 vs_color;

out vec4 fs_color;

uniform Material material;

void main()
{
    fs_color = vs_color;
}
