#pragma once

#include "Upgrade.h"
#include "global.h"
#include "Text.h"

namespace scene
{
	class BulletAccel_Up : public Upgrade
	{
	public:
		BulletAccel_Up(glm::mat4& matrix, Shader* _shader)
			: Upgrade(matrix, _shader)
		{
			Cube::texture = scene_textures::plus_bullet_accel;
		}

		virtual ~BulletAccel_Up()
		{

		}

		virtual void collectMe()
		{
			player::bullet_acceleration_plus += 10.0f;

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -3, -1);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.04, 0.04, 0.04));

			text->setText("+ Bullet Acceleration!");

			text->activateFade(1);

			hud::hud.push_back(text);

			Mix_PlayChannel(-1, sounds::snd_gotItem, 0);
		}
	};
}