#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
in vec3 normal;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 worldNormal;
out vec3 fragVert;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

uniform mat4 model;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	
	fragVert = vertexPosition_modelspace;
	worldNormal = (model * vec4(normal,0)).xyz;
}

