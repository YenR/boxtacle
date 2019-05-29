#pragma once

#include "Upgrade.h"
#include "global.h"
#include "Text.h"

namespace scene
{
	class Healthpack : public Upgrade
	{
	public:
		Healthpack(glm::mat4& matrix, Shader* _shader)
			: Upgrade(matrix, _shader)
		{
			Cube::texture = scene_textures::healthpack;
		}

		virtual ~Healthpack()
		{

		}

		virtual void collectMe()
		{
			if (player::lives < 3)
				player::lives++;

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -1, -1);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

			text->setText("+ 1 Life!");

			text->activateFade(1);

			hud::hud.push_back(text);

			Mix_PlayChannel(-1, sounds::snd_gotItem, 0);
		}
	};
}