#pragma once

#include "Upgrade.h"
#include "global.h"
#include "Text.h"

namespace scene
{
	class MaxJumps_Up : public Upgrade
	{
	public:
		MaxJumps_Up(glm::mat4& matrix, Shader* _shader)
			: Upgrade(matrix, _shader)
		{
			Cube::texture = scene_textures::plus_max_jumps;
		}

		virtual ~MaxJumps_Up()
		{

		}

		virtual void collectMe()
		{
			player::max_jumps++;

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -1, -1);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

			text->setText("+ 1 Jump!");

			text->activateFade(1);

			hud::hud.push_back(text);

			Mix_PlayChannel(-1, sounds::snd_gotItem, 0);

		}
	};
}