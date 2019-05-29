#pragma once

#include "SceneObject.h"
#include <string>
#include <vector>
#include <iostream>
#include "Shader.h"
#include "Texture.h"
#include <glm\gtc\type_ptr.hpp>

// Tutorial for obj-File loading: http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

namespace scene {

	class objModel : public SceneObject
	{
	public:

		Shader *shader;
		Texture *texture;

		GLuint vertexbuffer;
		GLuint uvbuffer;
		GLuint normalbuffer;
		GLuint VertexArrayID;

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals; // Won't be used at the moment.

		objModel(glm::mat4& _modelMatrix, const std::string& pFile, Shader *_shader)
			:SceneObject(_modelMatrix), shader(_shader)
		{

			glGenVertexArrays(1, &VertexArrayID);
			glBindVertexArray(VertexArrayID);
			
			// Load the texture
			//GLuint Texture = loadDDS("uvmap.DDS");
			
			// Read our .obj file
			bool res = loadOBJ(pFile.c_str(), vertices, uvs, normals);

			// Load it into a VBO

			glGenBuffers(1, &vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

			glGenBuffers(1, &uvbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

			glGenBuffers(1, &normalbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			GLint normalIndex = glGetAttribLocation(shader->programHandle, "normal");
			if (normalIndex != -1)
			{
				glEnableVertexAttribArray(normalIndex);
				glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}

		virtual ~objModel()
		{
			glDeleteBuffers(1, &vertexbuffer);
			glDeleteBuffers(1, &uvbuffer);
			glDeleteBuffers(1, &normalbuffer);
			glDeleteVertexArrays(1, &VertexArrayID);
		}

		virtual void update(float time_delta)
		{

		}

		virtual void draw()
		{
			shader->useShader();

			auto model_location = glGetUniformLocation(shader->programHandle, "model");
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Compute the MVP matrix from keyboard and mouse input

			GLuint MatrixID = glGetUniformLocation(shader->programHandle, "MVP");

			glm::mat4 ProjectionMatrix = getProjMatrix();
			glm::mat4 ViewMatrix = getViewMatrix();
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			if (texture != nullptr)
			{
				texture->bind(0);
			}

			GLuint TextureID = glGetUniformLocation(shader->programHandle, "myTextureSampler");
			glUniform1i(TextureID, 0);

			glBindVertexArray(VertexArrayID);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			glBindVertexArray(0);
		}

	private:

		bool loadOBJ(
			const char * path,
			std::vector<glm::vec3> & out_vertices,
			std::vector<glm::vec2> & out_uvs,
			std::vector<glm::vec3> & out_normals
			){
			printf("Loading OBJ file %s...\n", path);

			std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
			std::vector<glm::vec3> temp_vertices;
			std::vector<glm::vec2> temp_uvs;
			std::vector<glm::vec3> temp_normals;


			FILE * file = fopen(path, "r");
			if (file == NULL){
				printf("Could not open file: %s\n", path);
				getchar();
				return false;
			}

			while (1){

				char lineHeader[128];
				// read the first word of the line
				int res = fscanf(file, "%s", lineHeader);
				if (res == EOF)
					break; // EOF = End Of File. Quit the loop.

				// else : parse lineHeader

				if (strcmp(lineHeader, "v") == 0){
					glm::vec3 vertex;
					fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
					temp_vertices.push_back(vertex);
				}
				else if (strcmp(lineHeader, "vt") == 0){
					glm::vec2 uv;
					fscanf(file, "%f %f\n", &uv.x, &uv.y);
					uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
					temp_uvs.push_back(uv);
				}
				else if (strcmp(lineHeader, "vn") == 0){
					glm::vec3 normal;
					fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
					temp_normals.push_back(normal);
				}
				else if (strcmp(lineHeader, "f") == 0){
					std::string vertex1, vertex2, vertex3;
					unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
					int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
					if (matches != 9){
						printf("File can't be read by our simple parser. Try exporting with other options\n");
						return false;
					}
					vertexIndices.push_back(vertexIndex[0]);
					vertexIndices.push_back(vertexIndex[1]);
					vertexIndices.push_back(vertexIndex[2]);
					uvIndices.push_back(uvIndex[0]);
					uvIndices.push_back(uvIndex[1]);
					uvIndices.push_back(uvIndex[2]);
					normalIndices.push_back(normalIndex[0]);
					normalIndices.push_back(normalIndex[1]);
					normalIndices.push_back(normalIndex[2]);
				}
				else{
					// Probably a comment, eat up the rest of the line
					char stupidBuffer[1000];
					fgets(stupidBuffer, 1000, file);
				}

			}

			// For each vertex of each triangle
			for (unsigned int i = 0; i<vertexIndices.size(); i++){

				// Get the indices of its attributes
				unsigned int vertexIndex = vertexIndices[i];
				unsigned int uvIndex = uvIndices[i];
				unsigned int normalIndex = normalIndices[i];

				// Get the attributes thanks to the index
				glm::vec3 vertex = temp_vertices[vertexIndex - 1];
				glm::vec2 uv = temp_uvs[uvIndex - 1];
				glm::vec3 normal = temp_normals[normalIndex - 1];

				// Put the attributes in buffers
				out_vertices.push_back(vertex);
				out_uvs.push_back(uv);
				out_normals.push_back(normal);

			}

			return true;
		}


	};

}