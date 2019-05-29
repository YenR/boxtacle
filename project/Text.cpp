#include "Text.h"

using namespace scene;

Text::Text(glm::mat4& matrix, Shader* _shader, float x, float y)
	:HUDObject(matrix, _shader, x, y)
{
	texture = scene_textures::text_texture;
	fading = false;
}


Text::~Text()
{
	for (int i = 0; i < letters.size(); i++)
	{
		delete letters[i];
	}

	letters.clear();
}


void Text::activateFade(float fadingtime)
{
	fading = true;
	fadeTimer = fadingtime;
}

void Text::update(float time_delta)
{
	if (fading)
		fadeTimer -= time_delta;

	if (fading && fadeTimer <= 0)
	{
		for (int i = 0; i < hud::hud.size(); i++)
		{
			if (hud::hud[i] == this)	// if text has faded -> remove it from hud objects
			{
				hud::hud.erase(hud::hud.begin() + i);
				//std::cout << "Deleted fading text!\n";
				delete this;
				return;
			}
		}
	}

	for (int i = 0; i < letters.size(); i++) {
		letters[i]->translation = translation;
		letters[i]->rotation = rotation;
		letters[i]->scale = scale;

		letters[i]->update(time_delta);
	}
}

void Text::draw()
{
	for (int i = 0; i < letters.size(); i++) {
		letters[i]->draw();
	}
}

void Text::setText(std::string c)
{
	Letter* letter;

	for (int i = 0; i < letters.size(); i++)
	{
		delete letters[i];
	}

	letters.clear();

	for (int i = 0; i < c.length(); i++) {
		letter = new Letter(glm::mat4(1.0f), shaders::tex_shader, x_offset+((float)i)/2.75, y_offset);
		letter->texture = texture;
		letter->setLetter(c.at(i));
		letters.push_back(letter);
		
	}
}


void Text::updateText(std::string c)
{
	for (int i = 0; i < c.length() && i < letters.size(); i++) {
		letters[i]->setLetter(c.at(i));
	}
}
