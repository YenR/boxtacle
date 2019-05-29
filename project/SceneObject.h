#pragma once

//#define SCENE_OBJECT 

#include <glm/glm.hpp>

namespace scene
{
	__declspec(align(16)) class SceneObject
	{
	public:
		SceneObject(glm::mat4& _modelMatrix);
		virtual ~SceneObject();

		virtual void update(float time_delta) = 0;
		virtual void draw() = 0;

		glm::mat4 modelMatrix;

		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_mm_free(p);
		}
	};
};