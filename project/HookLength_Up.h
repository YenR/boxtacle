#pragma once

#include "Upgrade.h"
#include "global.h"
#include "Text.h"

namespace scene
{
	class HookLengthUp : public Upgrade
	{
	public:
		HookLengthUp(glm::mat4& matrix, Shader* _shader)
			: Upgrade(matrix, _shader)
		{
			Cube::texture = scene_textures::plus_hook_length;
		}

		virtual ~HookLengthUp()
		{

		}

		virtual void collectMe()
		{
			player::hookshot_length += 1.0f;

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -2, -1);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

			text->setText("+ Hook Length!");

			text->activateFade(1);

			hud::hud.push_back(text);

			Mix_PlayChannel(-1, sounds::snd_gotItem, 0);
		}
	};
}