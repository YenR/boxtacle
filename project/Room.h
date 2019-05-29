#pragma once

#include <GL\glew.h>
#include "SceneObject.h"
#include "Shader.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Texture.h"
#include "Cube.h"
#include "Bullet.h"
#include "Upgrade.h"
#include <vector>
#include <sstream>
#include "glm\glm.hpp"
#include <iostream>

#include "Coin.h"
#include "Healthpack.h"
#include "Speed_Up.h"
#include "MaxJumps_Up.h"
#include "BulletAccel_Up.h"
#include "BulletSpeed_Up.h"
#include "JumpHeight_Up.h"
#include "ShootingRate_Up.h"
#include "HookLength_Up.h"
#include "FrustumG.h"

#include "Enemy.h"

namespace scene
{
	class Room : public SceneObject
	{
	public:
		Room(glm::mat4& _modelMatrix, int seed, int width, int height, int length, glm::vec4 &wallMaterialColor);

		virtual ~Room();
		virtual void update(float time_delta);
		virtual void draw();

		virtual void draw_zBuffer(bool drawBullets);

		void draw(FrustumG *frustum);
		void updateLights();

		scene::Cube *lightcube1, *lightcube2, *lightcube3, *lightcube4;

		// room walls
		scene::Cube *floorCube, *wallCube1, *wallCube2, *wallCube3, *wallCube4, *ceilingCube;

		std::vector<scene::Cube*> cubes;
		std::vector<scene::Bullet*> bullets;
		std::vector<scene::Bullet*> enemy_bullets;
		std::vector<scene::Upgrade*> upgrades;
		std::vector<scene::Enemy*> enemies;

		std::vector<Light> lights;
		std::vector<glm::mat4> light_view;
		glm::mat4 light_proj;

		int room_length;
		int room_width;
		int room_height;

		glm::vec3 goal_point;
		glm::vec3 start_point;

	private:


	};
}