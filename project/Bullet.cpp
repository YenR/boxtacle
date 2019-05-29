#include "Bullet.h"

using namespace scene;

Bullet::Bullet(glm::mat4& matrix, Shader* _shader, glm::vec3 _direction, float _speed, float _time_to_live, float acceleration)
	:Cube(matrix, _shader), direction(_direction), speed(_speed), time_to_live(_time_to_live), accel(acceleration)
{

}

Bullet::~Bullet()
{

}

void Bullet::update(float time_delta)
{
	time_to_live -= time_delta;
	speed += accel * time_delta;

	if (speed > 80)		// capped to guarantee collission detection (?)
		speed = 80;

	Cube::translation = glm::translate(Cube::translation, direction * time_delta * speed);
	Cube::update(time_delta);
}
