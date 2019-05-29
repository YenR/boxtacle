#include "Cube.h"
#include "SceneObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include "global.h"

using namespace scene;


Cube::Cube(glm::mat4& matrix, Shader* _shader)
	:SceneObject(matrix), shader(_shader)
{
	
	//Load Data to Buffer
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, CUBE_VERTEX_COUNT * 3 * sizeof(float), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, CUBE_INDEX_COUNT * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, CUBE_VERTEX_COUNT * 3 * sizeof(float), normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &uvBuffer);

	upload_uvs(uvs);
	
	//Generate bindings
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	GLint positionIndex = glGetAttribLocation(shader->programHandle, "position");
	if (positionIndex != -1)
	{
		glEnableVertexAttribArray(positionIndex);
		glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	shaders::zBufferShader->useShader();
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	positionIndex = glGetAttribLocation(shaders::zBufferShader->programHandle, "position");
	if (positionIndex != -1)
	{
		glEnableVertexAttribArray(positionIndex);
		glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	
	shader->useShader();
		
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	GLint normalIndex = glGetAttribLocation(shader->programHandle, "normal");
	if (normalIndex != -1)
	{
		glEnableVertexAttribArray(normalIndex);
		glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	GLint uvIndex = glGetAttribLocation(shader->programHandle, "uv");
	if (uvIndex != -1)
	{
		glEnableVertexAttribArray(uvIndex);
		glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	translation = glm::mat4(1.0f);
	rotation = glm::mat4(1.0f);
	scale = glm::mat4(1.0f);

	damage = 0.0f;
	texture = nullptr;
}


Cube::~Cube()
{
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &positionBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteBuffers(1, &normalsBuffer);
	glDeleteVertexArrays(1, &vao);
}

void Cube::upload_uvs(const float* new_uvs)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, CUBE_VERTEX_COUNT * 2 * sizeof(float), new_uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cube::update(float time_delta)
{
	modelMatrix = translation * rotation * scale;

	//modelMatrix = glm::rotate(modelMatrix, 1.0f * time_delta, glm::vec3(1, 1, 1));

	//modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f + time_delta, 1.0f + time_delta, 1.0f + time_delta));
	
	//modelMatrix = glm::translate(modelMatrix, glm::vec3(0.1f * time_delta, 0.1f * time_delta, 0.1f * time_delta));
}

/*
void Cube::update(float time_delta, glm::mat4& view_proj)
{
	update(time_delta);
	
	shader->useShader();

	auto view_proj_location = glGetUniformLocation(shader->programHandle, "view_proj");
	glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(view_proj));
}*/

void Cube::draw()
{
	shader->useShader();
	
	auto model_location = glGetUniformLocation(shader->programHandle, "model");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	GLint dmg_location = glGetUniformLocation(shader->programHandle, "damage");
	if (dmg_location != -1)
		glUniform1f(dmg_location, damage);
	
	GLint matLoc = glGetUniformLocation(shader->programHandle, "materialColor");
	if (matLoc != -1)
		glUniform4fv(matLoc, 1, glm::value_ptr(materialColor));

	if (texture != nullptr)
		texture->bind(0);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, CUBE_INDEX_COUNT, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// ?? not sure if needed
	glUseProgram(0);
}

void Cube::draw_zBuffer()
{
	shaders::zBufferShader->useShader();

	auto model_location = glGetUniformLocation(shaders::zBufferShader->programHandle, "model");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, CUBE_INDEX_COUNT, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

const float Cube::positions[CUBE_VERTEX_COUNT * 3] =
{
	//Back
	-0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,

	//Front
	-0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	//Top
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f,

	//Bottom
	0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,

	//Right
	0.5f, 0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, 0.5f, -0.5f,

	//Left
	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f
};

const unsigned Cube::indices[CUBE_INDEX_COUNT] =
{
	0, 1, 2,
	0, 2, 3,

	4, 5, 6,
	4, 6, 7,

	8, 9, 10,
	8, 10, 11,

	12, 13, 14,
	12, 14, 15,
	
	16, 17, 18,
	16, 18, 19,

	20, 21, 22,
	20, 22, 23
};

const float Cube::normals[CUBE_VERTEX_COUNT *3] =
{
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,

	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,

	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,

	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,

	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f

};

const float Cube::uvs[CUBE_VERTEX_COUNT * 2] =
{
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,

	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,

	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,

	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f
};