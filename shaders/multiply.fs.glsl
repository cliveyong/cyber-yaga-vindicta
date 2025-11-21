#version 330

uniform sampler2D map_texture;
uniform sampler2D light_texture;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main()
{
    color = texture(map_texture, texcoord) * texture(light_texture, texcoord);
}
