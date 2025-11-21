#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float glitch_remaining;
uniform float glitch_duration;

in vec2 texcoord;

layout(location = 0) out vec4 color;

// M1: creative element #1
// Creates chromatic aberration effect on-hit
// Adds vignette around the player and increases color contrast

void main()
{
    float elapsed_time = (glitch_duration - glitch_remaining) / glitch_duration;
    elapsed_time = clamp(elapsed_time, 0.0, 1.0);
    float t = smoothstep(0.0, 0.5, elapsed_time) * smoothstep(1.0, 0.5, elapsed_time);
    float offset = 0.015 * t;

    vec3 chromatic_color;
    chromatic_color.r = texture(screen_texture, texcoord + vec2(offset, offset)).r;
    chromatic_color.g = texture(screen_texture, texcoord).g;
    chromatic_color.b = texture(screen_texture, texcoord + vec2(-offset, -offset)).b;
	chromatic_color = pow(chromatic_color, vec3(1.5));

    color = vec4(chromatic_color, 1.0);
}