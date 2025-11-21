#version 330

in vec2 texcoord;

uniform sampler2D albedo;
uniform vec3 fcolor;
uniform float alpha;
uniform float time;
uniform float outline_transparency;
uniform vec3 outline_color_a;
uniform vec3 outline_color_b;

const float pulse_speed = 5.0;
const float pulse_min = 0.8;
const float pulse_max = 1.;
const float outline_width = 0.015;
const float alpha_threshold = 0.01;
const float edge_alpha_threshold = 0.5;

layout(location = 0) out vec4 color;

void main()
{
    vec4 texture_color = texture(albedo, texcoord);
    vec4 base_color = vec4(fcolor, alpha) * texture_color;
    
    color = base_color;
    
    if (outline_transparency <= 0.) return;
    
    float color_pulse = 0.5 + 0.5 * sin(time * 3.0);
    vec3 outline_color = mix(outline_color_a, outline_color_b, color_pulse);
    float pulse_intensity = pulse_min + (pulse_max - pulse_min) * (0.5 + 0.5 * sin(time * pulse_speed));
    
    if (texture_color.a < alpha_threshold) {
        float is_edge_bound = float(
            texcoord.x < outline_width || texcoord.x > 1.0 - outline_width || 
            texcoord.y < outline_width || texcoord.y > 1.0 - outline_width
        );
        
        if (is_edge_bound > 0.5) {
            color = vec4(0.0);
            return;
        }
        
        float edge_detection = 0.0;
        vec2 offsets[4] = vec2[4](
            vec2(outline_width, 0.0),
            vec2(-outline_width, 0.0),
            vec2(0.0, outline_width),
            vec2(0.0, -outline_width)
        );
        
        for (int i = 0; i < 4; i++) {
            vec2 sample_pos = texcoord + offsets[i];
            
            float in_bounds = step(0.0, sample_pos.x) * step(sample_pos.x, 1.0) * 
                             step(0.0, sample_pos.y) * step(sample_pos.y, 1.0);
            
            if (in_bounds > 0.5) {
                vec4 sample_color = texture(albedo, sample_pos);
                edge_detection += step(edge_alpha_threshold, sample_color.a);
            }
        }
        
        if (edge_detection > 0.0) {
            color = vec4(outline_color * pulse_intensity, outline_transparency);
        }
    }
}