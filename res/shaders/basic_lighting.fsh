#version 460 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in vec2 vs_uv;
in vec3 vs_normal;
in vec3 vs_pos;

out vec4 fs_color;

uniform Material material;

void main()
{
    vec4 texture_colour = texture(material.diffuse, vs_uv);
    if (texture_colour.a < 0.001) {
        discard;
    }

    fs_color = texture_colour;
}