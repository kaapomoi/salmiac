#version 460 core

in vec2 vs_uv;
in vec3 vs_normal;
in vec3 vs_pos;
in vec4 vs_color;

out vec4 fs_color;

uniform sampler2D atlas;

void main()
{
    float text_alpha = texture(atlas, vs_uv).r;
    if (text_alpha < 0.1){
        discard;
    }

    vec4 texture_color = vec4(1.0,1.0,1.0,text_alpha) * vs_color;

    fs_color = texture_color;
}
