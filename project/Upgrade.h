#pragma once

#include <GL\glew.h>
#include "SceneObject.h"
#include "Shader.h"
#include "Cube.h"

#include "SDL/SDL_mixer.h"


namespace scene
{
	class Upgrade : public Cube
	{
	public:
		Upgrade(glm::mat4& matrix, Shader* _shader);
		virtual ~Upgrade();

		virtual void update(float time_delta);
		virtual void collectMe();

	private:
		static const float uvs[CUBE_VERTEX_COUNT * 2];
	};
}