#pragma once

#include "Cube.h"

namespace scene
{
	class Bullet : public Cube
	{
	public:
		Bullet(glm::mat4& matrix, Shader* _shader, glm::vec3 _direction, float _speed, float _time_to_live, float acceleration);

		virtual ~Bullet();
		virtual void update(float time_delta);
		//virtual void update(float time_delta, glm::mat4& view_proj);
		
		float time_to_live;
		glm::vec3 direction;

	private:

		float accel;
		float speed;
	};
}