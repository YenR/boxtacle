#include "Room.h"
#include "global.h"
#include "SimpleEnemy.h"

using namespace scene;

Room::Room(glm::mat4& _modelMatrix, int seed, int width, int height, int length, glm::vec4 &wallMaterialColor)
	: SceneObject(_modelMatrix), room_width(width), room_height(height), room_length(length)
{

	// initiate lights
	Light light2, light3, light4, light5;
	
	light2.intensities = glm::vec3(0.3, 0.3, 0.3);
	light2.position = glm::vec3(room_width / 2 - 2, 4, room_length / 2 - 2);

	light3.intensities = glm::vec3(0.3, 0.3, 0.3);
	light3.position = glm::vec3(-(room_width / 2 - 2), 4, room_length / 2 - 2);

	light4.intensities = glm::vec3(0.3, 0.3, 0.3);
	light4.position = glm::vec3(room_width / 2 - 2, 4, -(room_length / 2 - 2));

	light5.intensities = glm::vec3(0.3, 0.3, 0.3);
	light5.position = glm::vec3(-(room_width / 2 - 2), 4, -(room_length / 2 - 2));

	lights.push_back(light2);
	lights.push_back(light3);
	lights.push_back(light4);
	lights.push_back(light5);

	// upload lights to shaders
	updateLights();

	// initiate light cubes
	glm::mat4 small_cube = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

	lightcube1 = new scene::Cube(small_cube, shaders::shader2);
	lightcube2 = new scene::Cube(small_cube, shaders::shader2);
	lightcube3 = new scene::Cube(small_cube, shaders::shader2);
	lightcube4 = new scene::Cube(small_cube, shaders::shader2);

	lightcube1->materialColor = glm::vec4(100, 100, 100, 1);
	lightcube2->materialColor = glm::vec4(100, 100, 100, 1);
	lightcube3->materialColor = glm::vec4(100, 100, 100, 1);
	lightcube4->materialColor = glm::vec4(100, 100, 100, 1);

	lightcube1->translation = glm::translate(glm::mat4(1), glm::vec3(room_width / 2 - 2, 4, room_length / 2 - 2));
	lightcube2->translation = glm::translate(glm::mat4(1), glm::vec3(-(room_width / 2 - 2), 4, room_length / 2 - 2));
	lightcube3->translation = glm::translate(glm::mat4(1), glm::vec3(room_width / 2 - 2, 4, -(room_length / 2 - 2)));
	lightcube4->translation = glm::translate(glm::mat4(1), glm::vec3(-(room_width / 2 - 2), 4, -(room_length / 2 - 2)));

	cubes.push_back(lightcube1);
	cubes.push_back(lightcube2);
	cubes.push_back(lightcube3);
	cubes.push_back(lightcube4);

	// initiate room walls
	// floor
	floorCube = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	floorCube->scale = glm::scale(floorCube->scale, glm::vec3(room_width - 1, 1, room_length - 1));
	floorCube->translation = glm::translate(floorCube->translation, glm::vec3(0, 0.01f, 0));
	cubes.push_back(floorCube);
	floorCube->materialColor = wallMaterialColor;

	// far side
	wallCube1 = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	wallCube1->scale = glm::scale(wallCube1->scale, glm::vec3(room_width - 1, room_height - 1, 1));
	wallCube1->translation = glm::translate(wallCube1->translation, glm::vec3(0, room_height / 2, -(room_length / 2 - 0.1)));
	cubes.push_back(wallCube1);
	wallCube1->materialColor = wallMaterialColor;

	// near side/behind
	wallCube2 = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	wallCube2->scale = glm::scale(wallCube2->scale, glm::vec3(room_width - 1, room_height - 1, 1));
	wallCube2->translation = glm::translate(wallCube2->translation, glm::vec3(0, room_height / 2, room_length / 2 - 0.1));
	cubes.push_back(wallCube2);
	wallCube2->materialColor = wallMaterialColor;

	// left wall
	wallCube3 = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	wallCube3->scale = glm::scale(wallCube3->scale, glm::vec3(1, room_height - 1, room_length - 1));
	wallCube3->translation = glm::translate(wallCube3->translation, glm::vec3(-(room_width / 2 - 0.1), room_height / 2, 0));
	cubes.push_back(wallCube3);
	wallCube3->materialColor = wallMaterialColor;

	// right wall
	wallCube4 = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	wallCube4->scale = glm::scale(wallCube4->scale, glm::vec3(1, room_height - 1, room_length - 1));
	wallCube4->translation = glm::translate(wallCube4->translation, glm::vec3(room_width / 2 - 0.1, room_height / 2, 0));
	cubes.push_back(wallCube4);
	wallCube4->materialColor = wallMaterialColor;

	//ceiling
	ceilingCube = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	ceilingCube->scale = glm::scale(ceilingCube->scale, glm::vec3(room_width - 1, 1, room_length - 1));
	ceilingCube->translation = glm::translate(ceilingCube->translation, glm::vec3(0, room_height - 0.1, 0));
	cubes.push_back(ceilingCube);
	ceilingCube->materialColor = wallMaterialColor;
}

Room::~Room()
{

	for (int i = 0; i < cubes.size(); i++)
	{
		delete cubes[i];
		cubes[i] = nullptr;
		std::cout << "deleted cube nr " << i << "\n";
	}

	for (int i = 0; i < upgrades.size(); i++)
	{
		delete upgrades[i];
		upgrades[i] = nullptr;
		std::cout << "deleted upgrade nr " << i << "\n";
	}

	for (int i = 0; i < bullets.size(); i++)
	{
		delete bullets[i];
		bullets[i] = nullptr;
		std::cout << "deleted bullet nr " << i << "\n";
	}

	for (int i = 0; i < enemy_bullets.size(); i++)
	{
		delete enemy_bullets[i];
		enemy_bullets[i] = nullptr;
		std::cout << "deleted enemy bullet nr " << i << "\n";
	}

	for (int i = 0; i < enemies.size(); i++)
	{
		delete enemies[i];
		enemies[i] = nullptr;
		std::cout << "deleted enemy nr " << i << "\n";
	}
}

void Room::updateLights()
{
	shaders::shader2->useShader();
	auto numLightsLoc = glGetUniformLocation(shaders::shader2->programHandle, "numLights");
	glUniform1i(numLightsLoc, lights.size());

	shaders::tex_shader2->useShader();
	numLightsLoc = glGetUniformLocation(shaders::tex_shader2->programHandle, "numLights");
	glUniform1i(numLightsLoc, lights.size());

	shaders::tex3dshader->useShader();
	numLightsLoc = glGetUniformLocation(shaders::tex3dshader->programHandle, "numLights");
	glUniform1i(numLightsLoc, lights.size());

	for (int i = 0; i < lights.size(); i++)
	{
		std::ostringstream pos, inten;
		pos << "lights[" << i << "].position";
		inten << "lights[" << i << "].intensities";

		shaders::shader2->useShader();

		auto lightPosLoc = glGetUniformLocation(shaders::shader2->programHandle, pos.str().c_str());
		glUniform3fv(lightPosLoc, 1, glm::value_ptr(lights[i].position));

		auto lightIntLoc = glGetUniformLocation(shaders::shader2->programHandle, inten.str().c_str());
		glUniform3fv(lightIntLoc, 1, glm::value_ptr(lights[i].intensities));

		shaders::tex_shader2->useShader();

		lightPosLoc = glGetUniformLocation(shaders::tex_shader2->programHandle, pos.str().c_str());
		glUniform3fv(lightPosLoc, 1, glm::value_ptr(lights[i].position));

		lightIntLoc = glGetUniformLocation(shaders::tex_shader2->programHandle, inten.str().c_str());
		glUniform3fv(lightIntLoc, 1, glm::value_ptr(lights[i].intensities));

		shaders::tex3dshader->useShader();

		lightPosLoc = glGetUniformLocation(shaders::tex3dshader->programHandle, pos.str().c_str());
		glUniform3fv(lightPosLoc, 1, glm::value_ptr(lights[i].position));

		lightIntLoc = glGetUniformLocation(shaders::tex3dshader->programHandle, inten.str().c_str());
		glUniform3fv(lightIntLoc, 1, glm::value_ptr(lights[i].intensities));
	}
}

void Room::update(float time_delta)
{
	// rotate lightcubes
	lightcube1->rotation = glm::rotate(lightcube1->rotation, time_delta, glm::vec3(0, 1, 0));
	lightcube2->rotation = glm::rotate(lightcube2->rotation, time_delta, glm::vec3(0, 1, 0));
	lightcube3->rotation = glm::rotate(lightcube3->rotation, time_delta, glm::vec3(0, 1, 0));
	lightcube4->rotation = glm::rotate(lightcube4->rotation, time_delta, glm::vec3(0, 1, 0));

	// upgrade collisions
	for (int i = 0; i < upgrades.size(); i++)
	{
		if (simple_collision_detection_for_upgrades(upgrades[i]->translation, player::position))
		{
			//std::cout << "\a";		// beep
			upgrades[i]->collectMe();

			// remove upgrade from collection
			scene::Upgrade *toDelete = upgrades[i];

			upgrades.erase(upgrades.begin() + i);

			delete toDelete;
		}
	}

	
	// create all model matrixes
	for (int i = 0; i < cubes.size(); i++)
		cubes[i]->update(time_delta);

	// update enemies
	for (int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->update(time_delta);
		
		// check if alive
		if (enemies[i]->hp <= 0)
		{
			// spawn upgrade
			// add new upgrade to scene

			scene::Upgrade *temp_u;

			int temp_random = rand() % 9;
			switch (temp_random)
			{
				case 0:
					temp_u = new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 1:
					temp_u = new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 2:
					temp_u = new scene::Speed_Up(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 3:
					temp_u = new scene::MaxJumps_Up(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 4:
					temp_u = new scene::BulletAccel_Up(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 5:
					temp_u = new scene::BulletSpeed_Up(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 6:
					temp_u = new scene::JumpHeight_Up(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 7:
					temp_u = new scene::ShootingRate_Up(glm::mat4(1.0f), shaders::tex_shader2);
					break;
				case 8:
					temp_u = new scene::HookLengthUp(glm::mat4(1.0f), shaders::tex_shader2);
					break;
			}

			temp_u->scale = glm::scale(temp_u->scale, glm::vec3(0.5f, 0.5f, 0.1f));
			temp_u->translation = enemies[i]->translation;

			upgrades.push_back(temp_u);

			// delete enemy
			scene::Enemy* enemy = enemies[i];
			enemies.erase(enemies.begin() + i);
			delete enemy;

			// spawn new enemy
			/*
			float rn = rand() % 5 + 1;

			enemy = new scene::SimpleEnemy(glm::mat4(1), 0.2, rn, 3);

			rn = rand() % 4 + 0.5;

			enemy->translation = glm::translate(enemy->translation, glm::vec3((rand() % (room_width - 4)) - (room_width - 4) / 2, 1 + ((float)rn)*0.3, 0));
			enemy->scale = glm::scale(enemy->scale, glm::vec3(rn, rn, rn));

			enemies.push_back(enemy);
			*/
		}
	}


	// delete dead bullets
	for (int i = 0; i < bullets.size(); i++)
	{
		if (bullets[i]->time_to_live <= 0)
		{
			scene::Bullet* bullet = bullets[i];

			bullets.erase(bullets.begin() + i);

			delete bullet;
			//std::cout << "bullet timed out!\n";
		}
	}

	for (int i = 0; i < enemy_bullets.size(); i++)
	{
		if (enemy_bullets[i]->time_to_live <= 0)
		{
			scene::Bullet* bullet = enemy_bullets[i];

			enemy_bullets.erase(enemy_bullets.begin() + i);

			delete bullet;
		}
	}

	// update all remaining bullets
	for (int i = 0; i < bullets.size(); i++)
	{
		bullets[i]->update(time_delta);

		// test for collision
		for (int j = 0; j < enemies.size(); j++)
		{
			if (simple_collision_detection(enemies[j]->translation, enemies[j]->scale, bullets[i]->modelMatrix))
			{
				// flag bullet for deletion
				bullets[i]->time_to_live = -1;
									
				// call enemy get hit function
				enemies[j]->getHit(player::bullet_damage_plus);
			}
			
		}

	}

	for (int i = 0; i < enemy_bullets.size(); i++)
	{
		enemy_bullets[i]->update(time_delta);

		// test for collision
		if (simple_collision_detection_for_upgrades(enemy_bullets[i]->translation, player::position))
		{
			// flag bullet for deletion
			enemy_bullets[i]->time_to_live = -1;

			// reduce lives
			player::lives -= 1;

			if (player::lives < 1)
			{
				// GAME OVER
				scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -1, -1);
				text->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

				text->setText("GAME OVER!");

				text->activateFade(1);

				hud::hud.push_back(text);

				text = new scene::Text(glm::mat4(1), shaders::tex_shader, -2, 0);
				text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

				text->setText("Press Esc to exit.");

				text->activateFade(1);

				hud::hud.push_back(text);

				Mix_PlayChannel(-1, sounds::snd_game_over, 0);

				gameOver = 10;
			}
		}
	}

	// update (rotate) all upgrades
	for (int i = 0; i < upgrades.size(); i++)
		upgrades[i]->update(time_delta);
}

void Room::draw()
{
	// draw bullets
	for (int i = 0; i < bullets.size(); i++) 
			bullets[i]->scene::Cube::draw();

	for (int i = 0; i < enemy_bullets.size(); i++)
		enemy_bullets[i]->scene::Cube::draw();
	
	// draw cubes
	for (int i = 0; i < cubes.size(); i++) 
			cubes[i]->draw();

	// draw enemies
	for (int i = 0; i < enemies.size(); i++)
		enemies[i]->draw();

	// draw upgrades
	for (int i = 0; i < upgrades.size(); i++)
		upgrades[i]->draw();
}

void Room::draw(FrustumG *frustum)
{
	objects_rendered = 0;

	// draw bullets
	for (int i = 0; i < bullets.size(); i++) {
		if (frustum->boxInFrustum(bullets[i]->translation, bullets[i]->scale)) {
			bullets[i]->scene::Cube::draw();
			objects_rendered++;
		}
	}
	for (int i = 0; i < enemy_bullets.size(); i++)
		if (frustum->boxInFrustum(enemy_bullets[i]->translation, enemy_bullets[i]->scale))
		{
			enemy_bullets[i]->scene::Cube::draw();
			objects_rendered++;
		}
	
	// draw cubes
	for (int i = 0; i < cubes.size(); i++) {
		if (frustum->boxInFrustum(cubes[i]->translation, cubes[i]->scale)) {
			cubes[i]->draw();
			objects_rendered++;
		}
	}
	// draw enemies
	for (int i = 0; i < enemies.size(); i++)
		if (frustum->boxInFrustum(enemies[i]->translation, enemies[i]->scale))
		{
			enemies[i]->draw();

			objects_rendered++;
		}

	// draw upgrades
	for (int i = 0; i < upgrades.size(); i++)
		if (frustum->boxInFrustum(upgrades[i]->translation, upgrades[i]->scale))
		{
		upgrades[i]->draw();
		objects_rendered++;
		}
}

void Room::draw_zBuffer(bool drawBullets)
{
	// draw bullets
	if (drawBullets)
	{
		for (int i = 0; i < bullets.size(); i++)
		{
			// draw object
			bullets[i]->scene::Cube::draw_zBuffer();
		}

		for (int i = 0; i < enemy_bullets.size(); i++)
		{
			// draw object
			enemy_bullets[i]->scene::Cube::draw_zBuffer();
		}
	}

	// draw upgrades
	for (int i = 0; i < upgrades.size(); i++)
	{
		upgrades[i]->draw_zBuffer();
	}

	// draw cubes
	for (int i = 0; i < cubes.size(); i++)
	{
		cubes[i]->draw_zBuffer();
	}

	// draw enemies
	for (int i = 0; i < enemies.size(); i++)
	{
		for (int j = 0; j < enemies[i]->parts.size(); j++)
		{
			enemies[i]->parts[j]->draw_zBuffer();
		}
	}
}