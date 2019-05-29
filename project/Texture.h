#pragma once

#include <string>
#include "GL\glew.h"

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void bind(int unit);

private:
	GLuint handle;
};