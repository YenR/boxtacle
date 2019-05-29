#pragma once

#include <GL\glew.h>
#include "2DObject.h"
#include "Shader.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Texture.h"



namespace scene
{
	class HUDObject : public _2DObject
	{
	public:
		HUDObject(glm::mat4& matrix, Shader* _shader, float x, float y);

		virtual ~HUDObject();
		virtual void update(float time_delta);
		virtual void draw();
		virtual void upload_uvs(const float* new_uvs);
		
		Texture *texture;

		glm::mat4 translation, rotation, scale;
		float x_offset, y_offset;
		Shader *shader;

	private:
		GLuint vao;
		GLuint positionBuffer, uvBuffer, indexBuffer;

		static const float positions[4 * 3];		// 4 points with 3 coordinates each
		static const float uvs[4 * 2];				// 4 points with 2 coordinates each
		static const unsigned int indices[3 * 2];	// 2 triangles with 3 points each

	};
}