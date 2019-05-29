#version 330 core

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec3 worldNormal;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view_proj;

void main()
{
	gl_Position = view_proj * model * vec4(position,1);
	worldNormal = (model * vec4(normal,0)).xyz;

	texCoord = uv;
}