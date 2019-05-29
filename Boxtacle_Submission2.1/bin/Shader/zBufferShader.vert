#version 330 core

in vec3 position;

uniform mat4 model;
uniform mat4 view_proj;

void main()
{
	gl_Position = view_proj * model * vec4(position,1);
}