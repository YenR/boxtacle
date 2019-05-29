#pragma once

#include "HUDObject.h"
namespace scene {
	class Letter : public scene::HUDObject
	{
	public:
		Letter(glm::mat4& matrix, Shader* _shader, float x, float y);
		~Letter();

		virtual void setLetter(char c);
	};
}
