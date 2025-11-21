#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D normal;
uniform sampler2D albedo;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 normal_color = texture(normal, vec2(texcoord.x, texcoord.y));
	vec4 texture_color = texture(albedo, texcoord);
	normal_color.a = texture_color.a;
	color = vec4(fcolor, 1.0) * normal_color;
}
