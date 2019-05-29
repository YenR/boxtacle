#pragma once

#include "Upgrade.h"
#include "global.h"
#include "Text.h"

namespace scene
{
	class BulletSpeed_Up : public Upgrade
	{
	public:
		BulletSpeed_Up(glm::mat4& matrix, Shader* _shader)
			: Upgrade(matrix, _shader)
		{
			Cube::texture = scene_textures::plus_bullet_speed;
		}

		virtual ~BulletSpeed_Up()
		{

		}

		virtual void collectMe()
		{
			player::bullet_speed_plus += 5.0f;

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -2, -1);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

			text->setText("+ Bullet Speed!");

			text->activateFade(1);

			hud::hud.push_back(text);

			Mix_PlayChannel(-1, sounds::snd_gotItem, 0);
		}
	};
}