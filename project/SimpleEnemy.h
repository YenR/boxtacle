#pragma once

#include "Enemy.h"
#include "global.h"
#include <iostream>

namespace scene
{
	class SimpleEnemy : public Enemy
	{
		public:

			Cube *head, *body, *left, *right;

			float shooting_cooldown;
			float max_shooting_cooldown;
			float movement_speed;
			float bullet_speed;

			float horizontal_angle;

			bool forward;

			SimpleEnemy(glm::mat4& matrix, float shoot_cooldown, float mvmnt_speed, float bllt_speed, glm::vec4 &baseColor)
				:Enemy(matrix, 5), max_shooting_cooldown(shoot_cooldown), movement_speed(mvmnt_speed), bullet_speed(bllt_speed)
			{

				head = new Cube(glm::mat4(1.0f), shaders::shader2);

				head->translation = glm::translate(head->translation, glm::vec3(0.8, 0.3, 0));
				head->scale = glm::scale(head->scale, glm::vec3(2, 0.2, 0.2));
				
				head->materialColor = baseColor + glm::vec4(0.3, 0.3, 0.1, 1);

				body = new Cube(glm::mat4(1.0f), shaders::shader2);
				body->scale = glm::scale(body->scale, glm::vec3(0.7, 0.7, 1));

				body->materialColor = baseColor + glm::vec4(0.3, 0.3, 0.5, 1);

				left = new Cube(glm::mat4(1.0f), shaders::shader2);
				left->translation = glm::translate(left->translation, glm::vec3(-0.4, -0.4, 0));
				left->scale = glm::scale(left->scale, glm::vec3(0.25, 0.25, 1.5));

				left->materialColor = baseColor + glm::vec4(0.1, 0.1, 0.5, 1);

				right = new Cube(glm::mat4(1.0f), shaders::shader2);
				right->translation = glm::translate(right->translation, glm::vec3(0.4, -0.4, 0));
				right->scale = glm::scale(right->scale, glm::vec3(0.25, 0.25, 1.5));

				right->materialColor = baseColor + glm::vec4(0.1, 0.1, 0.5, 1);

				parts.push_back(head);
				parts.push_back(body);
				parts.push_back(left);
				parts.push_back(right);
				
				forward = true;
				horizontal_angle = 0;
			}

			virtual void update(float time_delta)
			{
				// move
				if (forward)
					translation = glm::translate(translation, glm::vec3(0, 0, time_delta * movement_speed));
				else
					translation = glm::translate(translation, glm::vec3(0, 0, -time_delta * movement_speed));

				if (translation[3].z > player::room_length / 2 - 5)
					forward = false;
				else if (translation[3].z < -player::room_length / 2 + 5)
					forward = true;

				modelMatrix = translation * rotation * scale;

				// rotate head
				horizontal_angle += time_delta;
				head->rotation = glm::rotate(glm::mat4(1), horizontal_angle, glm::vec3(0, 1, 0));

				Enemy::update(time_delta);

				head->modelMatrix = head->rotation * head->translation * head->scale;
				head->modelMatrix = modelMatrix * head->modelMatrix;

				// shoot bullets
				shooting_cooldown -= time_delta;

				if (shooting_cooldown <= 0)
				{
					shoot_bullet();
					shooting_cooldown = max_shooting_cooldown;
				}
			}

			virtual void shoot_bullet()
			{

				glm::vec3 direction(
					sin(horizontal_angle + glm::radians(90.0)),
					0,
					cos(horizontal_angle + glm::radians(90.0))
					);

				// create new bullet, using bullet shader, facing the direction the enemy is facing, lasting 10 seconds, accelerating at 0 units per second

				scene::Bullet *new_bullet = new scene::Bullet(glm::mat4(1.0f), shaders::shader2, direction, bullet_speed, 10, -0.5);
				new_bullet->materialColor = glm::vec4(0.9, 0.2, 0.2, 1);

				new_bullet->scale = glm::scale(new_bullet->scale, glm::vec3(0.1f, 0.1f, 0.1f)) * scale;

				direction.x *= (1 + pow(scale[0].x, 1.5));
				direction.z *= (1 + pow(scale[2].z, 1.5));

				new_bullet->translation = translation;
				new_bullet->translation = glm::translate(new_bullet->translation, direction);
				new_bullet->translation = glm::translate(new_bullet->translation, glm::vec3(0, 0.3 * scale[1].y ,0));

				if (player::enemy_bullets != nullptr)
					player::enemy_bullets->push_back(new_bullet);

				
			}


			
	};


}