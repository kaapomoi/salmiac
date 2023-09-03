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
uniform int frame;

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( vec3 x )
{
    // The noise function returns a value in the range -1.0f -> 1.0f
    vec3 p = floor(x);
    vec3 f = fract(x);

    f       = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;

    return mix(mix(mix( hash(n+0.0), hash(n+1.0),f.x),
                   mix( hash(n+57.0), hash(n+58.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z)-.5;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec3 t = (float(frame)*vec3(1.0,1.0,0.5)/1.0)/1000.0;
    
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = vs_uv;
    uv=uv/4.0+.5;

    vec3 col = vec3(0.0);
    
    for(int i = 0; i < 16; i++){
        float i2 = float(i)*1.0;
        col.r+=noise(uv.xyy*(12.0+i2)+col.rgb+t*sign(sin(i2/3.0)));
        col.g+=noise(uv.xyx*(12.0+i2)+col.rgb+t*sign(sin(i2/3.0)));
        col.b+=noise(uv.yyx*(12.0+i2)+col.rgb+t*sign(sin(i2/3.0)));
    }

	 for(int i = 0; i < 16; i++){
        float i2 = float(i)*1.0;
        col.r+=noise(uv.xyy*(32.0)+col.rgb+t*sign(sin(i2/3.0)));
        col.g+=noise(uv.xyx*(32.0)+col.rgb+t*sign(sin(i2/3.0)));
        col.b+=noise(uv.yyx*(32.0)+col.rgb+t*sign(sin(i2/3.0)));
    }
    col.rgb/=32.0;
    col.rgb=mix(col.rgb,normalize(col.rgb)*2.0,1.0);
    col.rgb+=.3;
    
    vec3 hsv_col = rgb2hsv(col);
    
    hsv_col.x += 0.3 * sin(t.x);
    hsv_col.y *= 0.25;
    hsv_col.z *= 0.25;

    // Output to screen
    fs_color = vec4(hsv2rgb(hsv_col),1.0);
}
