#pragma once

#include "Enemy.h"
#include "global.h"
#include <iostream>


namespace scene
{
	class StationaryEnemy : public Enemy
	{
	public:

		Cube *head, *body, *bottom, *eye;

		float shooting_cooldown;
		float max_shooting_cooldown;
		float bullet_speed;

		float horizontal_angle;
		float vertical_angle;

		bool homing;

		StationaryEnemy(glm::mat4& matrix, float shoot_cooldown, float bllt_speed, glm::vec4 &baseColor, bool _homing)
			:Enemy(matrix, 10), max_shooting_cooldown(shoot_cooldown), bullet_speed(bllt_speed), homing(_homing)
		{

			head = new Cube(glm::mat4(1.0f), shaders::shader2);

			head->translation = glm::translate(head->translation, glm::vec3(0, 0.35, 0));
			head->scale = glm::scale(head->scale, glm::vec3(0.4, 0.4, 0.4));

			head->materialColor = baseColor + glm::vec4(0.3, 0.3, 0.1, 1);

			body = new Cube(glm::mat4(1.0f), shaders::shader2);
			body->translation = glm::translate(body->translation, glm::vec3(0, -0.15, 0));
			body->scale = glm::scale(body->scale, glm::vec3(0.3, 0.5, 0.3));

			body->materialColor = baseColor + glm::vec4(0.3, 0.3, 0.5, 1);

			bottom = new Cube(glm::mat4(1.0f), shaders::shader2);
			bottom->translation = glm::translate(bottom->translation, glm::vec3(0, -0.4, 0));
			bottom->scale = glm::scale(bottom->scale, glm::vec3(1, 0.2, 1));

			bottom->materialColor = baseColor + glm::vec4(0.1, 0.1, 0.5, 1);

			eye = new Cube(glm::mat4(1.0f), shaders::shader2);
			eye->translation = glm::translate(eye->translation, glm::vec3(0.1, 0.35, 0.0));
			eye->scale = glm::scale(eye->scale, glm::vec3(0.45, 0.2, 0.2));

			eye->materialColor = baseColor + glm::vec4(10.0, 0.0, 0.0, 1);
			
			parts.push_back(head);
			parts.push_back(body);
			parts.push_back(bottom);
			parts.push_back(eye);

			horizontal_angle = 0;
			vertical_angle = 0;
		}

		virtual void update(float time_delta)
		{
			modelMatrix = translation * rotation * scale;

			// rotate head
			if (!homing)
				horizontal_angle += time_delta;
			else
			{
				glm::vec3 direction;

				direction = (player::position - glm::vec3(0, 0.5, 0)) - glm::vec3(translation[3].x, translation[3].y, translation[3].z);
				direction = glm::normalize(direction);

				horizontal_angle = -asin(direction.z / (sqrt(pow(direction.z, 2) + pow(direction.x, 2))));

				if (direction.x <= 0)
				{
					horizontal_angle *= -1;
					horizontal_angle -= glm::radians(180.0);
				}

				vertical_angle = asin(direction.y);
			}

			head->rotation = glm::rotate(glm::mat4(1), horizontal_angle, glm::vec3(0, 1, 0));
			head->rotation = glm::rotate(head->rotation, vertical_angle, glm::vec3(0, 0, 1));
			eye->rotation = glm::rotate(glm::mat4(1), horizontal_angle, glm::vec3(0, 1, 0));
			eye->rotation = glm::rotate(eye->rotation, vertical_angle, glm::vec3(0, 0, 1));

			Enemy::update(time_delta);

			head->modelMatrix = head->rotation * head->translation * head->scale;
			head->modelMatrix = modelMatrix * head->modelMatrix;

			eye->modelMatrix = eye->rotation * eye->translation * eye->scale;
			eye->modelMatrix = modelMatrix * eye->modelMatrix;

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
			glm::vec3 direction;

			if (!homing)
			{
				direction = glm::vec3(
					sin(horizontal_angle + glm::radians(90.0)),
					0,
					cos(horizontal_angle + glm::radians(90.0))
					);
			}
			else
			{
				direction = (player::position - glm::vec3(0, 0.5, 0)) - glm::vec3(translation[3].x, translation[3].y, translation[3].z);
				direction = glm::normalize(direction);
			}


			scene::Bullet *new_bullet = new scene::Bullet(glm::mat4(1.0f), shaders::shader2, direction, bullet_speed * 2, 7, -1);
			new_bullet->materialColor = glm::vec4(2.0, 0.2, 0.2, 1);

			new_bullet->scale = glm::scale(new_bullet->scale, glm::vec3(0.1f, 0.1f, 0.1f)) * scale;

			//direction.x *= (1 + pow(scale[0].x, 1.5));
			//direction.z *= (1 + pow(scale[2].z, 1.5));

			new_bullet->translation = translation;
			new_bullet->translation = glm::translate(new_bullet->translation, direction);
			new_bullet->translation = glm::translate(new_bullet->translation, glm::vec3(0, 0.3 * scale[1].y, 0));

			if (player::enemy_bullets != nullptr)
				player::enemy_bullets->push_back(new_bullet);


		}



	};


}