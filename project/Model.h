#pragma once

#include <GL\glew.h>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include "SceneObject.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/PostProcess.h"
#include "assimp/Scene.h"
#include "Shader.h"
#include "Texture.h"
#include <IL\il.h>
#include "FrustumG.h"


// Tutorial used:
// http://www.lighthouse3d.com/cg-topics/code-samples/importing-3d-models-with-assimp/

namespace scene {
	class Model : public SceneObject
	{
	public:
		struct MyMesh{
			GLuint buffer;
			GLuint vao;
			GLuint texIndex;
			GLuint uniformBlockIndex;
			int numFaces;

			MyMesh();
			MyMesh(aiMesh *mesh, Shader* _shader);
			~MyMesh();
			void render();

		};

		// scale factor for the model to fit in the window
		float scaleFactor;
		std::vector<MyMesh*> myMeshes;

		Assimp::Importer importer;
		const aiScene* scene = nullptr;

		// images / texture
		// map image filenames to textureIds
		// pointer to texture Array
		std::map<std::string, GLuint> textureIdMap;

		Texture *texture;


		// Vertex Attribute Locations
		GLuint vertexLoc = 0, normalLoc = 1, texCoordLoc = 2;

		bool Import3DFromFile(const std::string& pFile);

		Model(glm::mat4& _modelMatrix, Shader* _shader);
		virtual ~Model();
		void get_bounding_box_for_node(const aiNode* nd,
			aiVector3D* min,
			aiVector3D* max);
		void get_bounding_box(aiVector3D* min, aiVector3D* max);
		void genVAOsAndUniformBuffer(const aiScene *sc);
		int LoadGLTextures(const aiScene* scene);

		void recursive_render(const aiScene *sc, const aiNode* nd);

		virtual void draw();
		void draw(FrustumG *frustum);
		virtual void update(float time_delta);

		void update_min_max_nodes();

	private:
		Shader *shader;
		glm::vec3 minNode, maxNode;
	};
}

