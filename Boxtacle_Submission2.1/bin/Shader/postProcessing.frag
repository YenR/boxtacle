#version 330 core

layout(location=0) out vec4 outColor;

in vec3 worldNormal;
in vec2 texCoord;
in vec2 image_size;

uniform sampler2D color_texture;

void main()
{
	outColor = texture2D(color_texture, texCoord);
}