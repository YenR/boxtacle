#pragma once
#include "HUDObject.h"
#include <iostream>
#include <string>
#include <vector>
#include "Letter.h"
#include "Shader.h"
#include "global.h"


namespace scene {
	class Text : public scene::HUDObject
	{
	public:
		Text(glm::mat4& matrix, Shader* _shader, float x, float y);
		~Text();

		virtual void setText(std::string c);
		virtual void draw();
		virtual void update(float time_delta);

		virtual void updateText(std::string c);

		void activateFade(float fadingtime);
	
		bool fading;
		float fadeTimer;

	private:
		
		std::vector<scene::Letter*> letters;
	};
}