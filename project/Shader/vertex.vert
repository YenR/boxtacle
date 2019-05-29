#version 330 core

#define MAX_LIGHTS 5


in vec3 position;
in vec3 normal;
in vec2 uv;


out vec3 worldNormal;
out float dmg;
out vec2 texCoord;
out vec3 fragVert;
out vec3 camera_pos;

out vec4 ShadowCoord[MAX_LIGHTS];

uniform mat4 model;
uniform mat4 view_proj;
uniform float damage;
uniform vec3 camera_position;
uniform int numLights;

uniform int simpleDraw;
uniform mat4 DepthVP[MAX_LIGHTS];

void main()
{
		mat4 biasMatrix = mat4(	vec4(0.5, 0.0, 0.0, 0.0),
								vec4(0.0, 0.5, 0.0, 0.0),
								vec4(0.0, 0.0, 0.5, 0.0),
								vec4(0.5, 0.5, 0.5, 1.0));

		gl_Position = view_proj * model * vec4(position,1);
		worldNormal = (model * vec4(normal,0)).xyz;
		dmg = damage;
		texCoord = uv;
		fragVert = position;
		camera_pos = camera_position;
		
	if(simpleDraw == 0)
	{
		for(int i = 0; i< numLights; i++)
		{
			ShadowCoord[i] = (biasMatrix * (DepthVP[i] * model)) * vec4(position, 1);
		}
	
	}
}