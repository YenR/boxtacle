#include "Letter.h"

using namespace scene;

Letter::Letter(glm::mat4& matrix, Shader* _shader, float x, float y)
	:HUDObject(matrix, _shader, x, y)
{

}


Letter::~Letter()
{
}

void Letter::setLetter(char c)
{
	float uv_x = (c % 16) / 16.0f;
	float uv_y = (c / 16) / 16.0f;

	float uvs[8] = { uv_x + 1.0f / 16.0f, 1.0f - uv_y,
		uv_x, 1.0f - uv_y,
		uv_x + 1.0f / 16.0f, 1.0f - (uv_y + 1.0f / 16.0f),
		uv_x, 1.0f - (uv_y + 1.0f / 16.0f) };

	HUDObject::upload_uvs(uvs);
}

