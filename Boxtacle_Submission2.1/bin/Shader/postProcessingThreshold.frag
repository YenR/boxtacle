#version 330 core

layout(location=0) out vec4 outColor;

in vec3 worldNormal;
in vec2 texCoord;
in vec2 image_size;

uniform sampler2D color_texture;

void main()
{
	vec4 color = texture2D(color_texture, texCoord);

	if( float(color.r + color.g + color.b) /3 > 0.8)
		outColor = color;
	else
		outColor = vec4(0,0,0,0);

}