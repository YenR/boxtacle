#version 330 core

layout(location=0) out vec4 outColor;

in vec3 worldNormal;
in vec2 texCoord;

uniform sampler2D color_texture;

void main()
{

	//if( texCoord.x < 0.01 || texCoord.x > 0.99 || texCoord.y < 0.01 || texCoord.y > 0.99)
	//{
	//	outColor = vec4(0,0,0, 1);
	//}
	//else
	//{
		outColor = vec4(texture(color_texture, texCoord).bgra);
	//}
}