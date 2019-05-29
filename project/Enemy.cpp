#include "Enemy.h"
#include <iostream>

using namespace scene;

Enemy::Enemy(glm::mat4& matrix, float max_hp)
	:SceneObject(matrix), hp(max_hp)
{

}

Enemy::~Enemy()
{
	for (int i = 0; i < parts.size(); i++)
	{
		delete parts[i];
		parts[i] = nullptr;
	}
}

void Enemy::update(float time_delta)
{
	for (int i = 0; i < parts.size(); i++)
	{
		parts[i]->update(time_delta);
		
		parts[i]->modelMatrix = modelMatrix * parts[i]->modelMatrix;

		// reset damage
		if (parts[i]->damage > 0)
		{
			parts[i]->damage -= time_delta;
			if (parts[i]->damage < 0)
				parts[i]->damage = 0;
		}
	}
}

void Enemy::draw()
{
	for (int i = 0; i < parts.size(); i++)
		parts[i]->draw();
}

void Enemy::update(float time_delta, glm::mat4& view_proj)
{

}

void Enemy::getHit(float dmg)
{
	for (int i = 0; i < parts.size(); i++)
	{
		parts[i]->damage = 1;
	}

	hp -= dmg;

	//std::cout << "Enemy got hit. Remaining HP: " << hp << "\n";
}