#version 330

layout(location = 0) in vec3 in_position; // Attribute
uniform vec2 light_pos;
uniform mat3 projection;

void main()
{
	vec2 pos = in_position.xy;
	if (in_position.z > 0.) {
		vec2 dis = pos - light_pos;
		pos += dis/sqrt(dis.x * dis.x + dis.y * dis.y) * 10000.;
	}
	vec3 projected_pos = projection * vec3(pos.xy, 1.f);
	gl_Position = vec4(projected_pos.xy, 0.0, 1.0);
	pos = projected_pos.xy;
}