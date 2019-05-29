#version 330 core

layout(location=0) out vec4 outColor;

in vec3 worldNormal;

void main()
{
	outColor = vec4((worldNormal.x +1) *0.3 , 0, 1, 1);
}