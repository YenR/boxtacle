#include "Shader.h"

#include <iostream>
#include <fstream>

using namespace std;

Shader::Shader(const std::string& vertexShader, const std::string& fragmentShader) : programHandle(0), vertexHandle(0), fragmentHandle(0)
{
	programHandle = glCreateProgram();

	if (programHandle == 0)
	{
		std::cout << "Failed to create shader program" << std::endl;
		system("PAUSE");
		exit(-1);
	}

	loadShader(vertexShader, GL_VERTEX_SHADER, vertexHandle);
	loadShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentHandle);

	link();
}

Shader::~Shader()
{
	glDeleteProgram(programHandle);
	glDeleteShader(vertexHandle);
	glDeleteShader(fragmentHandle);

}

void Shader::useShader() const
{
	glUseProgram(programHandle);
}


void Shader::loadShader(const std::string& shader, GLenum shaderType, GLuint& handle)
{
	std::ifstream shaderFile(shader);

	if (!shaderFile.good())
	{
		std::cout << "Error: Load file " << shader << std::endl;
		system("PAUSE");
		exit(-1);
	}
	
	// (1) read file into string
	std::string code = std::string(std::istreambuf_iterator<char>(shaderFile), std::istreambuf_iterator<char>());
	shaderFile.close();

	// (2) generate shader handle
	handle = glCreateShader(shaderType);

	if (handle == 0)
	{
		std::cout << "Failed to create shader" << std::endl;
		system("PAUSE");
		exit(-1);
	}

	// (3) compile shader
	auto codePtr = code.c_str();
	glShaderSource(handle, 1, &codePtr, NULL);
	glCompileShader(handle);

	// Testing for compilation errors
	GLint succeeded;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &succeeded);

	if (succeeded == GL_FALSE || !glIsShader(handle))
	{
		// read log and output it to std::cout
		GLint logSize;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);
		auto message = new char[logSize];

		glGetShaderInfoLog(handle, logSize, NULL, message);
		std::cout << "Failed to compile shader" << std::endl;
		std::cout << message << std::endl;
		system("PAUSE");
		delete[] message;
		exit(-1);
	}

}

void Shader::link()
{
	glAttachShader(programHandle, vertexHandle);
	glAttachShader(programHandle, fragmentHandle);

	glBindFragDataLocation(programHandle, 0, "fragColor");

	glLinkProgram(programHandle);

	// check for errors
	GLint succeded;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &succeded);

	if (!succeded)
	{
		// output errors
		GLint logSize;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logSize);

		auto message = new char[logSize];
		glGetProgramInfoLog(programHandle, logSize, NULL, message);

		std::cout << "Failed to link shader program" << std::endl;
		std::cout << message << std::endl;
		system("PAUSE");

		delete[] message;
		exit(-1);
	}
}

