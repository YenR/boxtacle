#pragma once

#include <string>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

class Shader
{
public:

	Shader(const std::string& vertexShader, const std::string& fragmentShader);
	~Shader();

	void useShader() const;

	GLuint programHandle;

private:

	void loadShader(const std::string& shader, GLenum shaderType, GLuint& handle);
	void link();

	GLuint vertexHandle;
	GLuint fragmentHandle;
};