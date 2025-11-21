#version 330

in vec2 pos;
in vec2 texcoord;

uniform float u_time;
uniform vec2 light_pos;
uniform float intensity;
uniform float radius;
uniform vec3 light_color;
uniform bool is_local = false;
uniform float height;

uniform sampler2D normal_map;
uniform sampler2D shadow_map;

const float intense_light_offset = 1000.f;

layout(location = 0) out  vec4 color;

void main()
{
    vec2 dis = pos - light_pos;
    float dis_length = length(dis);
    
    vec3 norm = normalize(texture(normal_map, texcoord).rgb - 0.5);
    vec3 light_dir = normalize(vec3(-dis.x, -dis.y, height));
    float norm_str = dot(norm, light_dir);
    
    float attenuation = max(0.0, 1.0 - dis_length / radius);
    vec3 non_local_color = (attenuation * intensity * light_color) * norm_str;
    
    float str = 1.0 / (sqrt(dis.x * dis.x + dis.y * dis.y + intense_light_offset * intense_light_offset) - intense_light_offset);
    vec3 local_color = (str * norm_str) * light_color;
    
    float selector = float(is_local);
    color = texture(shadow_map, texcoord) * vec4(mix(non_local_color, local_color, selector), 1.0);
}
