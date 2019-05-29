#pragma once

#include "Upgrade.h"
#include "global.h"
#include "Text.h"

namespace scene
{
	class Coin : public Upgrade
	{
	public:
		Coin(glm::mat4& matrix, Shader* _shader)
			: Upgrade(matrix, _shader)
		{
			Cube::texture = scene_textures::coinTexture;
		}

		virtual ~Coin()
		{

		}

		virtual void collectMe()
		{
			player::coins_collected++;

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -1, -1);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

			text->setText("+ 1 Coin!");

			text->activateFade(1);

			hud::hud.push_back(text);

			updateCoins();

			Mix_PlayChannel(-1, sounds::snd_gotCoin, 0);
		}
	};
}