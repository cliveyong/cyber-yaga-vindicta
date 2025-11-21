#version 330 core
layout (location = 0) in vec4 vertex;	// vec4 = vec2 pos (xy) + vec2 tex (zw)
out vec2 TexCoords;

uniform mat3 projection;

void main()
{
	TexCoords = vec2(vertex.z, vertex.w);
	vec3 pos = projection * vec3(vertex.x, vertex.y, 1.0);
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}