#pragma once

#include <GL\glew.h>
#include "SceneObject.h"
#include "Cube.h"
#include <vector>


namespace scene
{
	class Enemy : public SceneObject
	{
	public:
		Enemy(glm::mat4& matrix, float max_hp);

		virtual ~Enemy();
		virtual void update(float time_delta);
		virtual void update(float time_delta, glm::mat4& view_proj);
		virtual void draw();

		virtual void getHit(float dmg);

		float hp;

		glm::mat4 translation, rotation, scale;
		std::vector<scene::Cube*> parts;
	};
}