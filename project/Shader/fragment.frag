#version 330 core

layout(location=0) out vec4 outColor;

in vec3 worldNormal;
in float dmg;
in vec2 texCoord;

void main()
{
	if( texCoord.x < 0.05 || texCoord.x > 0.95 || texCoord.y < 0.05 || texCoord.y > 0.95)
	{
		outColor = vec4(0,0,0, 1);
	}
	else
	{
		outColor = vec4((worldNormal.x+1)*0.5 + dmg, (worldNormal.y+1)*0.5 - dmg, (worldNormal.z+1)*0.5 - dmg,  1);
	}
}