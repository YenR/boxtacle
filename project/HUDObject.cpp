#include "HUDObject.h"
#include "SceneObject.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace scene;


HUDObject::HUDObject(glm::mat4& matrix, Shader* _shader, float x, float y)
	:_2DObject(matrix), shader(_shader), x_offset(x), y_offset(y)
{

	//Load Data to Buffer
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 2 * sizeof(float), uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
	
	texture = nullptr;

	translation = glm::mat4(1.0f);
	rotation = glm::mat4(1.0f);
	scale = glm::mat4(1.0f);
}

HUDObject::~HUDObject()
{
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &positionBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vao);
}

void HUDObject::update(float time_delta)
{
	modelMatrix = translation * rotation * scale;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-x_offset, -y_offset, 0));
}


void HUDObject::draw()
{
	shader->useShader();

	auto model_location = glGetUniformLocation(shader->programHandle, "model");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	
	if (texture != nullptr)
		texture->bind(0);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

void HUDObject::upload_uvs(const float* new_uvs)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 2 * sizeof(float), new_uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

const float HUDObject::positions[4 * 3] =
{
	-0.5f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f
};

const unsigned HUDObject::indices[3 * 2] =
{
	0, 1, 2,
	2, 1, 3
};

const float HUDObject::uvs[4 * 2] =
{
	1.0f, 1.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f
};