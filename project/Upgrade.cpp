
#include "Upgrade.h"

using namespace scene;

Upgrade::Upgrade(glm::mat4& matrix, Shader* _shader)
	:Cube(matrix, _shader)
{
	Cube::upload_uvs(Upgrade::uvs);
}

Upgrade::~Upgrade()
{

}

void Upgrade::update(float time_delta)
{
	Cube::rotation = glm::rotate(Cube::rotation, time_delta, glm::vec3(0, 1, 0));

	Cube::update(time_delta);
}

void Upgrade::collectMe()
{
	// what happens when Upgrade is collected
}


const float Upgrade::uvs[CUBE_VERTEX_COUNT * 2] =
{
	1.0f / 3.0f, 0.5f,
	1.0f / 3.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.5f,

	2.0f / 3.0f, 0.5f,
	1.0f, 0.5f,
	1.0f, 1.0f,
	2.0f / 3.0f, 1.0f,

	1.0f / 3.0f, 0.5f,
	1.0f / 3.0f, 0.0f,
	2.0f / 3.0f, 0.0f,
	2.0f / 3.0f, 0.5f,

	2.0f / 3.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 0.5f,
	2.0f / 3.0f, 0.5f,
	
	0.0f, 0.5f,
	0.0f, 0.0f,
	1.0f / 3.0f, 0.0f,
	1.0f / 3.0f, 0.5f,

	2.0f / 3.0f, 1.0f,
	1.0f / 3.0f, 1.0f,
	1.0f / 3.0f, 0.5f,
	2.0f / 3.0f, 0.5f
};