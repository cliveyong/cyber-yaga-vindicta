#version 330

in vec3 in_position;

uniform mat3 inverse_projection;

out vec2 pos;
out vec2 texcoord;

void main()
{
    gl_Position = vec4(in_position.xy, 0, 1.0);
	pos = (inverse_projection * vec3(in_position.xy, 1.0)).xy;
	texcoord = (in_position.xy + 1) / 2.f;
}