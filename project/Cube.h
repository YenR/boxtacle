#pragma once

#include <GL\glew.h>
#include "SceneObject.h"
#include "Shader.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Texture.h"

#define CUBE_VERTEX_COUNT 24
#define CUBE_INDEX_COUNT 36

namespace scene
{
	class Cube : public SceneObject
	{
	public:
		Cube(glm::mat4& matrix, Shader* _shader);

		virtual ~Cube();
		virtual void update(float time_delta);
		//virtual void update(float time_delta, glm::mat4& view_proj);
		virtual void draw();

		virtual void draw_zBuffer();

		glm::mat4 translation, rotation, scale;

		Texture *texture;

		float damage;

		void upload_uvs(const float* new_uvs);

		glm::vec4 materialColor;
		Shader *shader;

	private:
		GLuint vao;
		GLuint positionBuffer, normalsBuffer, uvBuffer, indexBuffer, dmgBuffer;

		static const float positions[CUBE_VERTEX_COUNT * 3];
		static const float normals[CUBE_VERTEX_COUNT * 3];
		static const float uvs[CUBE_VERTEX_COUNT * 2];
		static const unsigned int indices[CUBE_INDEX_COUNT];

	};
}