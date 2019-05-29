#include <iostream>
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "Shader.h"
#include "SceneObject.h"
#include "Cube.h"
#include "Bullet.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <sstream>
#include "Texture.h"
#include <vector>
#include "HUDObject.h"
#include "Coin.h"
#include <ctime>
#include <chrono>
#include <thread>
#include "global.h"
#include "Healthpack.h"
#include "Speed_Up.h"
#include "MaxJumps_Up.h"
#include "BulletAccel_Up.h"
#include "BulletSpeed_Up.h"
#include "JumpHeight_Up.h"
#include "ShootingRate_Up.h"
#include "Room.h"
#include "Letter.h"
#include "Text.h"
#include <iomanip>
#include "SimpleEnemy.h"
#include "StationaryEnemy.h"
#include "FrustumG.h"
#include "objModel.h"

#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "Model.h"
//
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla 

#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2main.lib" )
#pragma comment( lib, "SDL2_mixer.lib" )


int width = 1280;
int height = 720;
bool fullscreen = false;
bool flying_debug = false;		// toggled via F6

bool use_shadowmapping = true;	// toggled via F10
bool draw_shadowmaps = false;	// toggled via F11

double fps_lock = 60.0;			// max fps (not always accurate at high numbers)
bool fps_locked = false;		// whether or not fps are locked to the number above. can be toggled via F7
bool fps_display = false;		// whether ot not fps are displayed and debug information is printed to console. can be toggled via F2

bool wireFrame = false;			// wireframe display, toggled via F3
bool transparency = true;		// whether or not transparency is turned on, toggled via F9

int texSampleQual = 1;			// Texture Sampling Quality (0 = Nearest Neighbor, 1 = Bilinear), toggled via F4
int mipMapQual = 2;				// Mip Mapping Quality (0 = Off, 1 = Nearest Neighbor, 2 = Linear), toggled via F5

int gameOver = 0;

void init(GLFWwindow* window);
void update(GLFWwindow* window, float deltaT);
void draw();
void cleanup();

glm::mat4 proj;
glm::mat4 view;
glm::mat4 view_proj;

glm::mat4 getViewMatrix()
{
	return view;
}

glm::mat4 getProjMatrix()
{
	return proj;
}

// cubes
//scene::Cube* obj;		// colorful cube in the middle of the room
//scene::Cube* cube2;		// questionable cube, needed for textures to work

// hud stuff
scene::Cube* armcannon;
Texture *crosshair_texture, *danbo_texture, *life_texture, *life_lost_texture;
scene::HUDObject *crosshair, *danbo, *life1, *life2, *life3;
scene::Text *fps, *coins;
namespace hud
{
	std::vector<scene::HUDObject*> hud;
}

// rooms
scene::Room *main_room, *safe_room;

// duck
//scene::Model *model;
scene::objModel *duck;

// Post processing stuff
GLuint fbo, fbo_texture, rbo_depth;
GLuint bloomFbo, bloomFbo_texture, bloomRbo_depth;
GLuint smallFbo, smallFbo_texture, smallRbo_depth;
Shader *ppShader, *thresholdShader, *blurShader;

int bloomReductionFactor = 6;

scene::HUDObject *screen;

// Shadow mapping stuff

#define MAXLIGHTS 5

GLuint sm_depth[MAXLIGHTS], sm_fb[MAXLIGHTS];

int shadowMappingRectionFactor = 2;

//View Frustum Culling
FrustumG frustum;

// GLOBAL STUFF
int objects_rendered = 0;

/**
* Uses translation and scale of an object to create an axis aligned cube for collision detection
* Then tests if the center of the bullet is in said cube, returning true if thats the case.
*/
bool simple_collision_detection(glm::mat4 &translation, glm::mat4 &scale, glm::mat4 &bullet)
{
	//std::cout << "X: " << scale[0].x << " Y: " << scale[1].y << " Z: " << scale[2].z << "\n";
	
	//float size = glm::max(glm::max(scale[0].x, scale[1].y), scale[2].z) * 0.5f;

	if (bullet[3].z > translation[3].z - scale[2].z * 0.5 && bullet[3].z < translation[3].z + scale[2].z * 0.5 &&
		bullet[3].x > translation[3].x - scale[0].x * 0.5 && bullet[3].x < translation[3].x + scale[0].x * 0.5 &&
		bullet[3].y > translation[3].y - scale[1].y * 0.5 && bullet[3].y < translation[3].y + scale[1].y * 0.5)
	{
		//std::cout << "Detected collision!\n";
		return true;
	}

	return false;
}

/**
* Used for upgrade collision, gives upgrades a 1x1 axis aligned cube as hitbox.
*/
bool simple_collision_detection_for_upgrades(glm::mat4 &translation, glm::vec3 &player_position)
{
	if (player_position.y > translation[3].y - 0.5f && player_position.y - 0.5 < translation[3].y + 0.5f &&
		player_position.x > translation[3].x - 0.5f && player_position.x < translation[3].x + 0.5f &&
		player_position.z > translation[3].z - 0.5f && player_position.z < translation[3].z + 0.5f)
	{
		return true;
	}

	return false;
}

// returns true if the player is directly above the given object, false if the player is anywhere else
bool simple_collision_detection_for_falling(glm::mat4 &translation, glm::mat4 &scale, glm::vec3 &player_position)
{
	float object_top = translation[3].y + scale[1].y * 0.5;
	float object_x_offset = scale[0].x * 0.5;
	float object_z_offset = scale[2].z * 0.5;

	if (player_position.x > translation[3].x - object_x_offset && player_position.x < translation[3].x +  object_x_offset
		&& player_position.z > translation[3].z - object_z_offset && player_position.z < translation[3].z + object_z_offset
		&& player_position.y - 1 > object_top)
		return true;

	return false;
}

scene::Cube* add_platform(glm::vec4 &color, glm::vec3 &translation, glm::vec3 &scale) {
	scene::Cube *platform = new scene::Cube(glm::mat4(1), shaders::shader2);
	platform->materialColor = color;
	platform->translation = glm::translate(platform->translation, translation);
	platform->scale = glm::scale(platform->scale, scale);
	return platform;
}

scene::Upgrade* add_upgrade(scene::Upgrade *upgrade, glm::vec3 &translation) {
	upgrade->translation = glm::translate(upgrade->translation, translation);
	upgrade->scale = glm::scale(upgrade->scale, glm::vec3(0.5f, 0.5f, 0.1f));;
	return upgrade;
}

namespace shaders
{
	Shader* shader;			// damage shader (not used anymore)
	Shader* shader2;		// material shader
	Shader* tex_shader;		// 2d texture shader
	Shader* tex_shader2;	// 3d texture shader

	Shader* tex3dshader;	// shader for obj models

	Shader* zBufferShader;	// shader for shadow mapping
}

namespace scene_textures
{
	Texture *texture, *texture2, *coinTexture,
		*healthpack, *plus_speed, *plus_max_jumps,
		*plus_shoot_rate, *plus_jump_momentum, 
		*plus_bullet_speed, *plus_bullet_accel, *plus_hook_length;

	Texture *text_texture;

	Texture *duck_texture;
}

scene::Text *pressE, *pressQ;

namespace player	// player stats/values/options
{
	// (BASEVALUE) means that the value can be modified by picking up upgrades

	int level = 1;

	bool allow_move = true;							// whether player is allowed to move or not
	bool hooking = false;							// whether the player is currently moving automatically after a successful hookshot

	float hookshot_length = 2.0f;					// distance hookshot travels 
	float hookshot_cooldown = 0;
	scene::Bullet *hookshot_head = nullptr;
	scene::Cube *hookshot_body;

	scene::Room *currentRoom = nullptr;				// pointer to current room
	int room_width = 50;							// dimensions of current room
	int room_height = 20;
	int room_length = 50;
	std::vector<scene::Bullet*> *enemy_bullets;		// enemy bullets in current room

	int lives = 3;									// current lifes (hearts) (BASEVALUE)
	float horizontalAngle = 0;						// horizontal view angle
	float verticalAngle = 0;						// vertical view angle
	float speed = 3;								// movement speed (BASEVALUE)
	float mouse_speed = 1;							// mouse acceleration

	glm::vec3 position;								// position in world

	float input_cooldown = 0;						// used for changing options via F-Keys
	float shoot_cooldown = 0;						// used when shooting
	float upwards_momentum = 0;						// used when jumping
	float jump_cooldown = 0;						// neccessary because holding space uses up all jumps otherwise

	float shoot_cooldown_value = 0.4f;				// actual value in seconds between shots (BASEVALUE)
	int max_jumps = 2;								// = 1 normal + 0 air jumps (BASEVALUE)
	int jump_counter = 0;							// counts current amount of jumps
	float jump_momentum = 1.0f;						// added momentum on jumps (BASEVALUE)

	int coins_collected = 0;						// number of coins collected

	float bullet_speed_plus = 0.0f;					// added bullet speed (BASEVALUE)
	float bullet_acceleration_plus = 0.0f;			// added bullet acceleration (BASEVALUE)
	float bullet_damage_plus = 1.0f;				// added bullet damage (onto enemies) (BASEVALUE)
}

namespace sounds
{
	// Music initializing
	Mix_Music *m_Music;

	//The sound effects that will be used
	Mix_Chunk *snd_gotItem;
	Mix_Chunk *snd_gotCoin;
	Mix_Chunk *snd_game_over;
	Mix_Chunk *snd_level_complete;
}

// for joystick controls
bool using_joystick = false;
int axes, buttons;
const float* joystick_axes;
const unsigned char* joystick_buttons;

// for debugging with amd gpu
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);

static void APIENTRY DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
	std::string error = FormatDebugOutput(category, category, id, severity, message);
	std::cout << error << std::endl;
}

void upload_vp(glm::mat4 &vp)
{
	/*
	shaders::shader->useShader();
	auto view_proj_location = glGetUniformLocation(shaders::shader->programHandle, "view_proj");
	glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(view_proj));
	*/

	shaders::shader2->useShader();
	auto view_proj_location = glGetUniformLocation(shaders::shader2->programHandle, "view_proj");
	glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(vp));
	
	shaders::tex_shader2->useShader();
	view_proj_location = glGetUniformLocation(shaders::tex_shader2->programHandle, "view_proj");
	glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(vp));
}

void updateCoins()
{
	std::ostringstream cn;
	cn <<  "Coins: " << player::coins_collected;
	coins->updateText(cn.str());
}

void setTextureSamplingQuality(GLint value)
{

	for (int i = 0; i < hud::hud.size(); i++)
	{
		if (hud::hud[i] != screen)
		{
			hud::hud[i]->texture->bind(0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);
		}
	}

	scene_textures::coinTexture->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::duck_texture->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::healthpack->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_bullet_accel->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_bullet_speed->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_hook_length->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_jump_momentum->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_max_jumps->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_shoot_rate->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);

	scene_textures::plus_speed->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);
}

// main method - contains opengl init and main loop
int main(int argc, char** argv)
{
	if (argc >= 3)
	{
		if ((std::stringstream(argv[1]) >> width).fail())
		{
			std::cout << "Error reading first parameter" << std::endl;
			system("PAUSE");
			exit(-1);
		}

		if ((std::stringstream(argv[2]) >> height).fail())
		{
			std::cout << "Error reading second parameter" << std::endl;
			system("PAUSE");
			exit(-1);
		}

		if (argc >= 4)
		{
			if ((std::stringstream(argv[3]) >> fullscreen).fail())
			{
				std::cout << "Error reading third parameter" << std::endl;
				system("PAUSE");
				exit(-1);
			}
		}

	}

	if (!glfwInit())
	{
		std::cout << "Failed to init glfw" << std::endl;
		system("PAUSE");
		exit(-1);
	}

	#if _DEBUG
		// Create a debug OpenGL context or tell your OpenGL library (GLFW, SDL) to do so.
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#endif

	// (2) set window hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor *monitor = nullptr;
	if (fullscreen)
	{
		monitor = glfwGetPrimaryMonitor();

		int refresh_rate = 60;
		glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);
	}

	// (3) open window
	GLFWwindow* window = glfwCreateWindow(width, height, "Hello World!", monitor, nullptr);
	if (!window)
	{
		glfwTerminate();
		std::cout << "Failed to open window" << std::endl;
		system("PAUSE");
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	
	// (4) init glew
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize glew" << std::endl;
		system("PAUSE");
		exit(-1);
	}
	
	bool running = true;
	double deltaT;
	double time = glfwGetTime();
	double lastTime = time;

	#if _DEBUG
		// Query the OpenGL function to register your callback function.
		PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)glfwGetProcAddress("glDebugMessageCallbackAMD");

		if (_glDebugMessageCallbackAMD != NULL) {
			_glDebugMessageCallbackAMD(DebugCallbackAMD, NULL);
		}

	#endif

	// clear open gl error thingy
	glGetError();
	
	// (1) init everything you need
	init(window);

	float fps_cooldown = 0.0f;

	while (running && !glfwWindowShouldClose(window))
	{

		double minFrameTime = 1.0 / fps_lock;

		// (2) clear the frame and depth buffer
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// (3) compute the frame time delta
		time = glfwGetTime();
		deltaT = time - lastTime;

		if (fps_locked && deltaT < minFrameTime)
		{
			//std::cout << "Sleeping to preserve fps, for: " << (int((minFrameTime - deltaT) * 1000)) << "ms" << std::endl;
			std::this_thread::sleep_for(std::chrono::microseconds(int((minFrameTime - deltaT) * 1000000)));

			time = glfwGetTime();
			deltaT = time - lastTime;
		}

		lastTime = time;

		// fps counter in console
		if (fps_display)
		{
			fps_cooldown -= deltaT;
			if (fps_cooldown <= 0)
			{
				std::ostringstream fpsnumber;
				fpsnumber << std::fixed << std::setw(2) << std::setprecision(2) << "FPS: " << 1.0 / (deltaT);
				fps->updateText(fpsnumber.str());

				std::cout << "frametime = " << deltaT << "s, fps: " << 1.0 / (deltaT) << ", Objects rendered: " << objects_rendered << "\n";
				
				fps_cooldown = 0.2f;
			}
		}

		// (4) react to user input
		glfwPollEvents();
		running = !glfwGetKey(window, GLFW_KEY_ESCAPE);

		// (5) Update all game components 
		if (glfwGetWindowAttrib(window, GLFW_FOCUSED) && gameOver != 1)		// only update when focused
			update(window, deltaT);

		if (gameOver > 1)
			gameOver--;

		// (6) draw all game components
		draw();
		glfwSwapBuffers(window);

		// (7) check for errors
		/*	// done via callback
		GLenum error = glGetError();

		if (error != GL_NO_ERROR) {
			switch (error) {
			case GL_INVALID_ENUM:
				std::cerr << "GL: enum argument out of range." << std::endl;
				break;
			case GL_INVALID_VALUE:
				std::cerr << "GL: Numeric argument out of range." << std::endl;
				break;
			case GL_INVALID_OPERATION:
				std::cerr << "GL: Operation illegal in current state." << std::endl;
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				std::cerr << "GL: Framebuffer object is not complete." << std::endl;
				break;
			case GL_OUT_OF_MEMORY:
				std::cerr << "GL: Not enough memory left to execute command." << std::endl;
				break;
			default:
				std::cerr << "GL: Unknown error." << std::endl;
			}
			
		}
		*/
	}

	// (8) clean up!
	cleanup();
}

void initLevel1()
{
	if (main_room != nullptr)
	{
		delete main_room;
		main_room = nullptr;
	}

	// create room (matrix for sceneobject, seed = 0, width = 50, height = 20, length = 50, wallcolor rgb = brown)
	main_room = new scene::Room(glm::mat4(1), 0, 50, 20, 50, glm::vec4(0.8, 0.5, 0.1, 1));
	
	// testing enemy
	scene::SimpleEnemy *simpleEnemy = new scene::SimpleEnemy(glm::mat4(1), 2.0, 3, 5, glm::vec4(0.2, 0.0, 0.0, 1.0));

	simpleEnemy->translation = glm::translate(simpleEnemy->translation, glm::vec3(-10, 1.5, 0));
	simpleEnemy->scale = glm::scale(simpleEnemy->scale, glm::vec3(2, 2, 2));

	main_room->enemies.push_back(simpleEnemy);

	simpleEnemy = new scene::SimpleEnemy(glm::mat4(1), 1.0, 3, 8, glm::vec4(0.0, 0.3, 0.3, 1.0));

	simpleEnemy->translation = glm::translate(simpleEnemy->translation, glm::vec3(15, 0.9, 0));
	simpleEnemy->scale = glm::scale(simpleEnemy->scale, glm::vec3(0.8, 0.8, 0.8));

	main_room->enemies.push_back(simpleEnemy);

	for (int i = 0; i < 5; i++)
	{
		scene::Cube *platform1 = new scene::Cube(glm::mat4(1), shaders::shader2);
		platform1->materialColor = glm::vec4(0.8, 0.5, 0.1, 1);
		platform1->translation = glm::translate(platform1->translation, glm::vec3(10, 5 - i, 15 - i * 3));
		platform1->scale = glm::scale(platform1->scale, glm::vec3(6 - i, 0.2, 6 - i));

		main_room->cubes.push_back(platform1);
	}

	scene::StationaryEnemy *stationaryEnemy = new scene::StationaryEnemy(glm::mat4(1), 0.5, 5, glm::vec4(0.0, 0.5, 0.0, 1.0), false);

	stationaryEnemy->translation = glm::translate(stationaryEnemy->translation, glm::vec3(10, 6.5, 15));
	stationaryEnemy->scale = glm::scale(stationaryEnemy->scale, glm::vec3(1.8, 2.2, 1.8));

	main_room->enemies.push_back(stationaryEnemy);

	//Level generation + fill coins
	scene::Upgrade* upgrade;
	scene::Cube* platform;
	
	// First platforms
	for (int i = 0; i < 6; i++) {
		main_room->cubes.push_back(add_platform(glm::vec4(1.8, 1.2, 0.1, 1), glm::vec3(22.5 - i * 6, 2 + 0.5f*i, -22.5), glm::vec3(3, 0.2, 3)));
		main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(22.5 - i * 6, 3 + 0.5f*i, -22.5)));
	}

	// second corner + healthpack
	main_room->cubes.push_back(add_platform(glm::vec4(0.8, 1.5, 1.1, 1), glm::vec3(-22.5, 2, -22.5), glm::vec3(3, 0.2, 3)));
	main_room->upgrades.push_back(add_upgrade(new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-22, 3, -22)));

	// blue collars
	for (int i = 0; i < 5; i++) {
		main_room->cubes.push_back(add_platform(glm::vec4(0, 1-i/2, 0.5+i/2, 1), glm::vec3(-22.5, 2, -18.5 + i * 5), glm::vec3(3, 3 + i * 2, 3)));
		main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(22.5 - i * 6, 3 + 0.5f*i, -22.5)));
	}

	// 3 orange collars
	main_room->cubes.push_back(add_platform(glm::vec4(1.8, 0.5, 0.1, 1), glm::vec3(-17, 4, -2.5), glm::vec3(2, 8, 2)));
	main_room->cubes.push_back(add_platform(glm::vec4(1.8, 0.5, 0.1, 1), glm::vec3(-17, 4, -7.5), glm::vec3(2, 8, 2)));
	main_room->cubes.push_back(add_platform(glm::vec4(1.8, 0.5, 0.1, 1), glm::vec3(-17, 4, -13.5), glm::vec3(2, 8, 2)));
	
	// yellow box
	main_room->cubes.push_back(add_platform(glm::vec4(2.8, 2.5, 0.1, 1), glm::vec3(-10, 8, -13.5), glm::vec3(10, 0.3f, 0.5f)));
	
	// 4 teal small platforms
	main_room->cubes.push_back(add_platform(glm::vec4(0, 2.5, 2.1, 1), glm::vec3(0, 8, -13.5), glm::vec3(2, 0.3f, 2)));
	main_room->upgrades.push_back(add_upgrade(new scene::MaxJumps_Up(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(0, 9, -13.5)));

	main_room->cubes.push_back(add_platform(glm::vec4(0, 2.5, 2.1, 1), glm::vec3(8, 6, -11.5f), glm::vec3(2, 0.3f, 2)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(8, 7, -11.5f)));

	main_room->cubes.push_back(add_platform(glm::vec4(0, 2.5, 2.1, 1), glm::vec3(16, 6, -15.5f), glm::vec3(2, 0.3f, 2)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(16, 7, -15.5f)));

	main_room->cubes.push_back(add_platform(glm::vec4(0, 2.5, 2.1, 1), glm::vec3(23, 6, -11.5f), glm::vec3(2, 0.3f, 2)));
	main_room->upgrades.push_back(add_upgrade(new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(23, 7, -11.5f)));


	main_room->cubes.push_back(add_platform(glm::vec4(0, 3, 0, 1), glm::vec3(23, 9, -7.5f), glm::vec3(2, 0.3f, 2)));
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(23, 13, -7), glm::vec3(1)));

	main_room->cubes.push_back(add_platform(glm::vec4(0, 3, 0, 1), glm::vec3(19, 13, -7.5f), glm::vec3(2, 0.3f, 2)));
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(18.5f, 17, -7.5f), glm::vec3(1)));

	main_room->cubes.push_back(add_platform(glm::vec4(4, 0.4, 4, 1), glm::vec3(15, 13, -7.5f), glm::vec3(0.5f, 0.3f, 4)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 0.4, 4, 1), glm::vec3(11, 13, -7.5f), glm::vec3(0.5f, 0.3f, 4)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 0.4, 4, 1), glm::vec3(7, 13, -7.5f), glm::vec3(0.5f, 0.3f, 4)));

	main_room->cubes.push_back(add_platform(glm::vec4(4, 3.8, 0, 1), glm::vec3(1, 13, -9.5f), glm::vec3(4, 0.3f, 0.5f)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 0.4, 4, 1), glm::vec3(-3, 14, -7), glm::vec3(0.5f, 0.3f, 6)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 3.8, 0, 1), glm::vec3(-7, 15, -5.5), glm::vec3(4, 0.3f, 0.5f)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 0.4, 4, 1), glm::vec3(-11, 16, -7), glm::vec3(0.5f, 0.3f, 6)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 3.8, 0, 1), glm::vec3(-15, 15, -9.5), glm::vec3(4, 0.3f, 0.5f)));

	main_room->cubes.push_back(add_platform(glm::vec4(3, 0, 0, 1), glm::vec3(-18, 12, -5.5), glm::vec3(3, 2, 3)));
	main_room->upgrades.push_back(add_upgrade(new scene::HookLengthUp(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-18, 14.5, -5.5)));

	main_room->cubes.push_back(add_platform(glm::vec4(3, 0, 0, 1), glm::vec3(-22.5, 16, 0), glm::vec3(3, 2, 3)));
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(-23, 19, 0.5), glm::vec3(1, 1, 1)));

	main_room->cubes.push_back(add_platform(glm::vec4(4, 3.8, 0, 1), glm::vec3(-19, 17, 5), glm::vec3(8, 0.3f, 0.5)));

	// stairs
	for (int i = 0; i < 10; i++) {
		main_room->cubes.push_back(add_platform(glm::vec4(3.0f-((float)i/5), 2.8f+((float)i/5), 0, 1), glm::vec3(-13, 15 - i*0.5, 5 - i*0.5), glm::vec3(8, 0.3f, 0.5)));
		if (i % 2 == 0)
			main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-10 - i*0.8, 16 - i*0.5, 5 - i*0.5)));
	}

	//platform under the stairs
	main_room->cubes.push_back(add_platform(glm::vec4(0, 3, 0, 1), glm::vec3(-13, 9, 2.5), glm::vec3(8, 0.3f, 7)));
	main_room->upgrades.push_back(add_upgrade(new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-10, 10, 4.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-13, 10, 4.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-16, 10, 4.5)));

	// after stairs
	main_room->cubes.push_back(add_platform(glm::vec4(0, 3, 0, 1), glm::vec3(-3, 9, 2.5), glm::vec3(2, 0.3f, 7)));
	main_room->upgrades.push_back(add_upgrade(new scene::ShootingRate_Up(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-3, 10, 2.5)));

	// Small cyan pods
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(0, 8, 2.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(0, 9, 2.5)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(1, 8, 5.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(1, 9, 5.5)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(1, 8, -1.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(1, 9, -1.5)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(3, 7.5, 2), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(3, 8.5, 2)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(4, 8, -1.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(4, 9, -1.5)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(5, 8, 6.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(5, 9, 6.5)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(7, 7, -1.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(7, 8, -1.5)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 4, 4, 1), glm::vec3(6.5, 6.5, 2.5), glm::vec3(1, 0.3f, 1)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(6.5, 7.5, 2.5)));

	main_room->cubes.push_back(add_platform(glm::vec4(4, 3.8, 0, 1), glm::vec3(13, 7, -1.5), glm::vec3(6, 0.3f, 1)));
	main_room->cubes.push_back(add_platform(glm::vec4(4, 3.8, 0, 1), glm::vec3(21, 7, -1.5), glm::vec3(6, 0.3f, 1)));

	// chess blocks red + purple
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			main_room->cubes.push_back(add_platform(glm::vec4(2 + i, 0.4, 2 - j, 1), glm::vec3(16 + i * 2 + (j % 2), 7, 3 + j * 3), glm::vec3(1, 0.3f, 3)));
			if (i % 2 == 1)
				main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(16 + i * 2 + (j % 2), 8, 3 + j * 3)));
		}
	}

	// 4 blue blocks in corner
	main_room->cubes.push_back(add_platform(glm::vec4(2, 2, 3, 1), glm::vec3(22.5, 1, 21), glm::vec3(3, 2, 6)));
	main_room->cubes.push_back(add_platform(glm::vec4(1.3, 1.3, 3, 1), glm::vec3(19.5, 3, 19.5), glm::vec3(3, 6, 3)));
	main_room->cubes.push_back(add_platform(glm::vec4(0.6, 0.6, 3, 1), glm::vec3(19.5, 5, 22.5), glm::vec3(3, 10, 3)));
	main_room->cubes.push_back(add_platform(glm::vec4(0, 0, 3, 1), glm::vec3(22.5, 12, 22.5), glm::vec3(3, 8, 3)));

	// coins
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(22.5, 2.5, 22.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(23.5, 2.5, 23.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(21.5, 2.5, 21.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(23.5, 2.5, 21.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(21.5, 2.5, 23.5)));
	main_room->upgrades.push_back(add_upgrade(new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(22.5, 2.5, 19.5)));

	// hookcubes
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(18.5, 8, 18.5), glm::vec3(1)));
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(18.5, 12, 23.5), glm::vec3(1)));
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(21.5, 18, 23.5), glm::vec3(1)));
	main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(5, 18, 22), glm::vec3(1)));

	// stairs 
	for (int i = 0; i < 7; i++) {
		main_room->cubes.push_back(add_platform(glm::vec4(0 + (float)i / 4, 3, (float)i / 4, 1), glm::vec3(-3.5 - i * 2, 7 - i, 22), glm::vec3(2, 0.3f, 3)));
		main_room->upgrades.push_back(add_upgrade(new scene::Coin(glm::mat4(1.0f), shaders::tex_shader2), glm::vec3(-3.5 - i * 2, 8 - i, 22)));

	}
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			if ((i + j) % 2 == 0)

				main_room->cubes.push_back(add_platform(glm::vec4(4), glm::vec3(-23.5 + i, 1, 20.5 + j), glm::vec3(1)));
			else
				main_room->cubes.push_back(add_platform(glm::vec4(0, 0, 0, 1), glm::vec3(-23.5 + i, 1, 20.5 + j), glm::vec3(1)));
		}
	}

	// set goal
	main_room->goal_point = glm::vec3(-20, 2, 23);

	// set position
	main_room->start_point = glm::vec3(21.5, 19, -23.5);

}

void initLevel2()
{
	if (main_room != nullptr)
	{
		delete main_room;
		main_room = nullptr;
	}

	main_room = new scene::Room(glm::mat4(1), 0, 30, 20, 30, glm::vec4(0.5, 0.6, 0.8, 1));

	main_room->start_point = glm::vec3(0, 1, 0);

	main_room->goal_point = glm::vec3(0, 1, 0);

}

void initLevel3()
{
	if (main_room != nullptr)
	{
		delete main_room;
		main_room = nullptr;
	}

	main_room = new scene::Room(glm::mat4(1), 0, 20, 20, 20, glm::vec4(0.2, 0.7, 0.4, 1));


	main_room->cubes.push_back(add_platform(glm::vec4(0.2, 0.7, 0.4, 1), glm::vec3(-8.5, 1.5, 0), glm::vec3(2, 2, 20)));
	main_room->cubes.push_back(add_platform(glm::vec4(0.2, 0.7, 0.4, 1), glm::vec3(8.5, 1.5, 0), glm::vec3(2, 2, 20)));

	main_room->cubes.push_back(add_platform(glm::vec4(0.2, 0.7, 0.4, 1), glm::vec3(0, 1.5, -8.5), glm::vec3(20, 2, 2)));
	main_room->cubes.push_back(add_platform(glm::vec4(0.2, 0.7, 0.4, 1), glm::vec3(0, 1.5, 8.5), glm::vec3(20, 2, 2)));


	scene::StationaryEnemy *stationaryEnemy = new scene::StationaryEnemy(glm::mat4(1), 3.0, 4, glm::vec4(0.1, 0.6, 0.3, 1.0), true);
	stationaryEnemy->translation = glm::translate(stationaryEnemy->translation, glm::vec3(2.5, 1, 0));
	main_room->enemies.push_back(stationaryEnemy);

	stationaryEnemy = new scene::StationaryEnemy(glm::mat4(1), 3.5, 4, glm::vec4(0.1, 0.6, 0.3, 1.0), true);
	stationaryEnemy->translation = glm::translate(stationaryEnemy->translation, glm::vec3(-2.5, 1, 0));
	main_room->enemies.push_back(stationaryEnemy);

	stationaryEnemy = new scene::StationaryEnemy(glm::mat4(1), 2.5, 4, glm::vec4(0.1, 0.6, 0.3, 1.0), true);
	stationaryEnemy->translation = glm::translate(stationaryEnemy->translation, glm::vec3(0, 1, 2.5));
	main_room->enemies.push_back(stationaryEnemy);

	stationaryEnemy = new scene::StationaryEnemy(glm::mat4(1), 4, 4, glm::vec4(0.1, 0.6, 0.3, 1.0), true);
	stationaryEnemy->translation = glm::translate(stationaryEnemy->translation, glm::vec3(0, 1, -2.5));
	main_room->enemies.push_back(stationaryEnemy);

	main_room->cubes.push_back(add_platform(glm::vec4(2, 2, 2, 1), glm::vec3(0, 0.35, 0), glm::vec3(1)));
	main_room->goal_point = glm::vec3(0, 1, 0);

	main_room->start_point = glm::vec3(0, 5 , -(main_room->room_length / 2 - 2));
}

void initSafeRoom()
{
	if (safe_room != nullptr)
	{
		delete safe_room;
		safe_room = nullptr;
	}

	float rr = rand() % 30, 
		rg = rand() % 30, 
		rb = rand() % 30;

	safe_room = new scene::Room(glm::mat4(1), 0, 20, 8, 20, glm::vec4(0.7 + rr/100, 0.7 + rg/100, 0.7 + rb/100, 1));

	for (int i = 0; i < 3; i++)
	{
		scene::Upgrade *temp_u;

		if (player::lives <= i)		// spawn enough healthpacks to get the player to max life
		{
			temp_u = new scene::Healthpack(glm::mat4(1.0f), shaders::tex_shader2);
		}
		else
		{
			int temp_random = rand() % 7 + 2;
			switch (temp_random)
			{
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
		}

		temp_u->scale = glm::scale(temp_u->scale, glm::vec3(0.5f, 0.5f, 0.1f));
		temp_u->translation = glm::translate(glm::mat4(1), glm::vec3((-3) + i * 3, 2, 0));

		safe_room->upgrades.push_back(temp_u);

	}

	safe_room->cubes.push_back(add_platform(glm::vec4(2, 2, 2, 1), glm::vec3(0, 0.35, (safe_room->room_length / 2 - 2)), glm::vec3(1)));

	safe_room->goal_point = glm::vec3(0, 1, (safe_room->room_length / 2 - 2));
	safe_room->start_point = glm::vec3(0, 1, -(safe_room->room_length / 2 - 2));
}

void switchRoom()
{
	player::input_cooldown = 0.3f;
	player::jump_counter = 0;

	if (player::currentRoom == safe_room)
	{
		switch (player::level % 3)
		{
		case 1: 
			initLevel1();
			break;

		case 2:
			initLevel2();
			break;

		case 0:
			initLevel3();
			break;
		}

		player::currentRoom = main_room;
		player::room_width = main_room->room_width;
		player::room_height = main_room->room_height;
		player::room_length = main_room->room_length;

		player::enemy_bullets = &(main_room->enemy_bullets);
	}
	else
	{
		player::level++;

		initSafeRoom();

		player::currentRoom = safe_room;
		player::room_width = safe_room->room_width;
		player::room_height = safe_room->room_height;
		player::room_length = safe_room->room_length;

		player::enemy_bullets = &(safe_room->enemy_bullets);

	}

	player::currentRoom->updateLights();

	// reset position
	player::horizontalAngle = 0;
	player::verticalAngle = 0;
	player::position = player::currentRoom->start_point;
}

void init(GLFWwindow* window)
{
	// init rng with current time
	std::srand(std::time(0));

	glfwSetWindowTitle(window, "Boxtacle v0.10b");

	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// enable depth testing (z buffering)
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

	// disable backface culling
	//glDisable(GL_CULL_FACE);

	// enable backface culling
	glEnable(GL_CULL_FACE);

	// set background/sky color = very dark brown
	glClearColor(0.01f, 0.005f, 0.0f, 0.0f);
	// set viewport
	glViewport(0, 0, width, height);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	// enable sticky keys (returns true next time getkeys is called if buttons was pressed, even if not pressed anymore)
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// create shaders
	ppShader = new Shader("Shader/postProcessing.vert", "Shader/postProcessing.frag");

	shaders::zBufferShader = new Shader("Shader/zBufferShader.vert", "Shader/zBufferShader.frag");

	thresholdShader = new Shader("Shader/postProcessing.vert", "Shader/postProcessingThreshold.frag");
	blurShader = new Shader("Shader/postProcessing.vert", "Shader/blurShader.frag");

	blurShader->useShader();
	auto size_location = glGetUniformLocation(blurShader->programHandle, "size");
	glUniform2f(size_location, float(width / bloomReductionFactor), float(height / bloomReductionFactor));

	shaders::shader = new Shader("Shader/vertex.vert", "Shader/fragment.frag");

	shaders::shader2 = new Shader("Shader/vertex.vert", "Shader/material.frag");

	shaders::tex_shader = new Shader("Shader/vertex.vert", "Shader/TextureShader.frag");

	shaders::tex_shader->useShader();
	auto tex_location = glGetUniformLocation(shaders::tex_shader->programHandle, "color_texture");
	glUniform1i(tex_location, 0);

	shaders::tex_shader2 = new Shader("Shader/vertex.vert", "Shader/TextureShader2.frag");

	shaders::tex_shader2->useShader();
	auto tex_location2 = glGetUniformLocation(shaders::tex_shader2->programHandle, "color_texture");
	glUniform1i(tex_location2, 0);

	shaders::tex3dshader = new Shader("Shader/3dObj.vert", "Shader/3dObj.frag");
	
	// set player position
	//player::position = glm::vec3(22.5, 4, -22.5);;

	// create new Object - rotating color cube
	//obj = new scene::Cube(glm::mat4(1.0f), shaders::shader);

	// translate it (put it 5 units up)
	//obj->translation = glm::translate(obj->translation, glm::vec3(0, 5, 0));

	// add it to current rooms objects to be drawn
	//player::currentRoom->cubes.push_back(obj);

	// add it to enemies to enable bullet collision and damage
	//player::currentRoom->enemies.push_back(obj);

	// add lightsource to the new cube
	//Light light1;
	//light1.intensities = glm::vec3(0.5, 0.5, 0.5);
	//light1.position = glm::vec3(0, 5, 0);
	//player::currentRoom->lights.push_back(light1);

	// arm cannon
	armcannon = new scene::Cube(glm::mat4(1.0f), shaders::shader2);
	armcannon->scale = glm::scale(armcannon->scale, glm::vec3(0.2f, 0.2f, 1.0f));
	armcannon->materialColor = glm::vec4(0.6, 0.3, 0.1, 1);

	// do not remove this piece of code. need it for textures to work. dont know why.
	// update: nvm
	/*{
		cube2 = new scene::Cube(glm::mat4(1.0f), shaders::tex_shader);
		cube2->translation = glm::translate(cube2->translation, glm::vec3(-1, -200, -1));

		scene_textures::texture = new Texture("Textures/test03.png");
		cube2->texture = scene_textures::texture;

		//cube2->draw();
		player::currentRoom->cubes.push_back(cube2);
		}*/

	// load textures for upgrades
	scene_textures::coinTexture = new Texture("Textures/coin-texture-nolines.png");
	scene_textures::healthpack = new Texture("Textures/healthpack.png");
	scene_textures::plus_speed = new Texture("Textures/plus-speed.png");
	scene_textures::plus_max_jumps = new Texture("Textures/plus-maxjumps.png");

	scene_textures::plus_shoot_rate = new Texture("Textures/plus-shootspeed.png");
	scene_textures::plus_jump_momentum = new Texture("Textures/plus-jumpheight.png");
	scene_textures::plus_bullet_speed = new Texture("Textures/plus-bulletspeed.png");
	scene_textures::plus_bullet_accel = new Texture("Textures/plus-bullet-accel.png");

	scene_textures::plus_hook_length = new Texture("Textures/plus-hook-length.png");

	scene_textures::duck_texture = new Texture("Textures/duck.png");


	// 2D objects
	{
		float aspect_ratio_offset = float(width) / float(height) - 0.1f;

		screen = new scene::HUDObject(glm::mat4(1), ppShader, 0, 0);
		screen->scale = glm::scale(screen->scale, glm::vec3(0.24f * (aspect_ratio_offset + 0.1f), 0.24f, 1));
		hud::hud.push_back(screen);

		crosshair_texture = new Texture("Textures/crosshair02.png");
		crosshair = new scene::HUDObject(glm::mat4(1.0f), shaders::tex_shader, 0.0f, 0.0f);

		crosshair->texture = crosshair_texture;
		crosshair->scale = glm::scale(crosshair->scale, glm::vec3(0.01f, 0.01f, 0.01f));
		hud::hud.push_back(crosshair);

		danbo_texture = new Texture("Textures/danbo.png");
		danbo = new scene::HUDObject(glm::mat4(1.0f), shaders::tex_shader, -2.65f * aspect_ratio_offset, -2.35f);

		danbo->texture = danbo_texture;
		danbo->scale = glm::scale(danbo->scale, glm::vec3(0.04f, 0.04f, 0.04f));
		hud::hud.push_back(danbo);

		life_texture = new Texture("Textures/life.png");
		life_lost_texture = new Texture("Textures/life-lost.png");
		life1 = new scene::HUDObject(glm::mat4(1.0f), shaders::tex_shader, 2.0f * (-2.65f + 0.8f / aspect_ratio_offset) * aspect_ratio_offset, -4.7f);

		life1->texture = life_texture;
		life1->scale = glm::scale(life1->scale, glm::vec3(0.02f, 0.02f, 0.02f));
		hud::hud.push_back(life1);

		life2 = new scene::HUDObject(glm::mat4(1.0f), shaders::tex_shader, 2.0f * (-2.65f + 1.3f / aspect_ratio_offset) * aspect_ratio_offset, -4.7f);

		life2->texture = life_texture;
		life2->scale = glm::scale(life2->scale, glm::vec3(0.02f, 0.02f, 0.02f));
		hud::hud.push_back(life2);

		life3 = new scene::HUDObject(glm::mat4(1.0f), shaders::tex_shader, 2.0f * (-2.65f + 1.8f / aspect_ratio_offset) * aspect_ratio_offset, -4.7f);

		life3->texture = life_texture;
		life3->scale = glm::scale(life3->scale, glm::vec3(0.02f, 0.02f, 0.02f));
		hud::hud.push_back(life3);

		scene_textures::text_texture = new Texture("Textures/font.png");
		fps = new scene::Text(glm::mat4(1.0f), shaders::tex_shader, 3.2f * aspect_ratio_offset, -5.0f);
		fps->scale = glm::scale(fps->scale, glm::vec3(0.02f, 0.02f, 0.02f));
		fps->setText("");
		hud::hud.push_back(fps);

		coins = new scene::Text(glm::mat4(1.0f), shaders::tex_shader, -10.0f * aspect_ratio_offset, -7.8f);
		coins->scale = glm::scale(coins->scale, glm::vec3(0.01f, 0.01f, 0.01f));
		coins->setText("Coins: 0   ");
		hud::hud.push_back(coins);
	}


	// init level 1

	main_room = nullptr;
	initLevel1();

	safe_room = nullptr;

	player::position = main_room->start_point;
	player::currentRoom = main_room;
	player::enemy_bullets = &(main_room->enemy_bullets);

	// update cube lights (load to shader)
	player::currentRoom->updateLights();

	pressE = new scene::Text(glm::mat4(1), shaders::tex_shader, -6, -0.5);
	pressE->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

	pressE->setText("Press E to advance to the next level!");

	pressQ = new scene::Text(glm::mat4(1), shaders::tex_shader, -6, 0.5);
	pressQ->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

	pressQ->setText("Press Q to buy a random powerup for 5 coins!");


	if (glfwJoystickPresent(GLFW_JOYSTICK_1))	// if joystick is used
	{
		using_joystick = true;
		joystick_axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes);
		joystick_buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons);

		std::cout << "Joystick detected! (axes: " << axes << ", buttons: " << buttons << ")\n";
	}

	player::hookshot_body = new scene::Cube(glm::mat4(1), shaders::shader2);
	player::hookshot_body->materialColor = glm::vec4(0.7, 0.7, 0.7, 0.7);
	player::hookshot_body->scale = glm::scale(glm::mat4(1), glm::vec3(0.03, 0.03, 100));

	// set cursor to middle of screen
	glfwSetCursorPos(window, double(width / 2), double(height / 2));

	proj = glm::perspective(glm::radians(60.0f), width / (float)height, 0.1f, 100.0f);
	frustum.setCamInternals(60.0f, width / (float)height, 0.1f, 100.0f);


	// load duck

	duck = new scene::objModel(glm::mat4(1), "Models/newDuck.obj", shaders::tex3dshader);

	duck->texture = scene_textures::duck_texture;

	duck->modelMatrix = glm::scale(glm::mat4(1), glm::vec3(2, 2, 2));
	duck->modelMatrix = glm::translate(duck->modelMatrix, glm::vec3(2, 0.5, 2));
	duck->modelMatrix = glm::rotate(duck->modelMatrix, float(glm::radians(150.0)), glm::vec3(0, 1, 0));

	/*
	model = new scene::Model(glm::mat4(1), shaders::tex_shader2);
	model->Import3DFromFile("Models/duck.dae");

	model->modelMatrix = glm::translate(glm::scale(glm::mat4(1), glm::vec3(2, 2, 2)), glm::vec3(0, 1, 2));

	model->texture = scene_textures::duck_texture;
	*/

	// Tutorial used for fbo: https://en.wikibooks.org/wiki/OpenGL_Programming/Post-Processing

	/* Create back-buffer, used for post-processing */
	/* Texture */
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &fbo_texture);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Depth buffer */
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	/* Framebuffer to link everything together */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
	GLenum status;
	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* Texture */
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &bloomFbo_texture);
	glBindTexture(GL_TEXTURE_2D, bloomFbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Depth buffer */
	glGenRenderbuffers(1, &bloomRbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, bloomRbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	/* Framebuffer to link everything together */
	glGenFramebuffers(1, &bloomFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomFbo_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloomRbo_depth);
	status;
	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* Texture */
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &smallFbo_texture);
	glBindTexture(GL_TEXTURE_2D, smallFbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width / bloomReductionFactor, height / bloomReductionFactor, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Depth buffer */
	glGenRenderbuffers(1, &smallRbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, smallRbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width / bloomReductionFactor, height / bloomReductionFactor);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	/* Framebuffer to link everything together */
	glGenFramebuffers(1, &smallFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, smallFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smallFbo_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, smallRbo_depth);
	status;
	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// for ShadowMapping 
	//(as per https://lva.cg.tuwien.ac.at/cgue/wiki/lib/exe/fetch.php?media=students:cgue_shadow_mapping.pdf)

	for (int i = 0; i < MAXLIGHTS; i++)
	{
		glActiveTexture(GL_TEXTURE1 + i);
		glGenTextures(1, &sm_depth[i]);
		glBindTexture(GL_TEXTURE_2D, sm_depth[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, height / shadowMappingRectionFactor, height / shadowMappingRectionFactor, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glGenFramebuffers(1, &sm_fb[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, sm_fb[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sm_depth[i], 0);
		glDrawBuffer(GL_NONE); // depth only
		//glReadBuffer(GL_NONE);

		std::ostringstream pos2;
		pos2 << "shadowMap[" << i << "]";

		shaders::shader2->useShader();
		auto texLoc = glGetUniformLocation(shaders::shader2->programHandle, pos2.str().c_str());
		//std::cout << "tex loc " << i << " " << texLoc << std::endl;
		glUniform1i(texLoc, i );

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "failed to create frame depth buffer for light " << i << "\n";
		}
	}

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, sm_depth[0]);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// View Frustum Culling enabling
	frustum.enabled = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());

	}
	
	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}

	//Load music & play
	sounds::m_Music = Mix_LoadMUS("sound/lava_music.ogg");
	if (sounds::m_Music == NULL)
	{
		printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
	}
	Mix_PlayMusic(sounds::m_Music, -1);

	//Load sound effects
	sounds::snd_game_over = Mix_LoadWAV("sound/game_over.ogg");
	if (sounds::snd_game_over == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
	}

	sounds::snd_gotCoin = Mix_LoadWAV("sound/got_coin.ogg");
	if (sounds::snd_gotCoin == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
	}

	sounds::snd_gotItem = Mix_LoadWAV("sound/got_item.ogg");
	if (sounds::snd_gotItem == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
	}

	sounds::snd_level_complete = Mix_LoadWAV("sound/level_complete.ogg");
	if (sounds::snd_level_complete == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
	}
}

//using namespace player;
glm::vec3 direction, up;

void update(GLFWwindow *window, float deltaT)
{

	// parts of movement taken from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/

	// check for F-commands
	if (glfwGetKey(window, GLFW_KEY_F6) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		flying_debug = !flying_debug;

		if (player::hookshot_head != nullptr)
		{
			// cancel hookshot

			delete player::hookshot_head;
			player::hookshot_head = nullptr;
			player::allow_move = true;
			player::hooking = false;
		}

		if (flying_debug)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Flying Mode: On");

			text->activateFade(1);

			hud::hud.push_back(text);
			
			player::speed += 30;
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Flying Mode: Off");

			text->activateFade(1);

			hud::hud.push_back(text);

			player::speed -= 30;
		}
	}
	/*
	if (glfwGetKey(window, GLFW_KEY_F1) && player::input_cooldown <= 0){

		// testing text on screen

		player::input_cooldown = 0.3f;

		scene::Text *help = new scene::Text(glm::mat4(1), shaders::tex_shader, (rand() % 6) - 3, (rand() % 6) - 3);
		help->scale = glm::scale(glm::mat4(1), glm::vec3(0.05, 0.05, 0.05));

		help->setText("HELP!");

		help->activateFade(1);

		hud::hud.push_back(help);
	}*/

	if (glfwGetKey(window, GLFW_KEY_F7) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		fps_locked = !fps_locked;

		if (fps_locked)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("FPS Lock: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("FPS Lock: Off");

			text->activateFade(1);

			hud::hud.push_back(text);
		}

		if (fps_display)
		{
			fps->setText("FPS:         ");
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F2) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		fps_display = !fps_display;
		
		if (fps_display)
		{
			fps->setText("FPS:         ");

			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("FPS Display: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("FPS Display: Off");

			text->activateFade(1);

			hud::hud.push_back(text);

			fps->setText("");
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F3) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		wireFrame = !wireFrame;

		if (wireFrame)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("WireFrame: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("WireFrame: Off");

			text->activateFade(1);

			hud::hud.push_back(text);
		}

	}

	if (glfwGetKey(window, GLFW_KEY_F9) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		transparency = !transparency;
		if (transparency)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Transparency: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Transparency: Off");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F11) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		draw_shadowmaps = !draw_shadowmaps;
		if (draw_shadowmaps)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Drawing Shadowmaps: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Drawing Shadowmaps: Off");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F10) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		use_shadowmapping = !use_shadowmapping;
		if (use_shadowmapping)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Shadowmapping: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Shadowmapping: Off");

			text->activateFade(1);

			hud::hud.push_back(text);


			for (int i = 0; i < MAXLIGHTS; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, sm_fb[i]);

				glClear(GL_DEPTH_BUFFER_BIT);
			}
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F8) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;
		frustum.enabled = !frustum.enabled;

		if (frustum.enabled)
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Frustum Culling: On");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
		else
		{
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Frustum Culling: Off");

			text->activateFade(1);

			hud::hud.push_back(text);
		}
	}


	if (glfwGetKey(window, GLFW_KEY_F4) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;

		if (texSampleQual == 0)
		{
			texSampleQual = 1;
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Texture Sampling Quality: Bilinear");

			text->activateFade(1);

			hud::hud.push_back(text);

			glActiveTexture(GL_TEXTURE0);

			if (mipMapQual == 0)
				setTextureSamplingQuality(GL_LINEAR);
			else if (mipMapQual == 1)
				setTextureSamplingQuality(GL_LINEAR_MIPMAP_NEAREST);
			else
				setTextureSamplingQuality(GL_LINEAR_MIPMAP_LINEAR);

		}
		else if (texSampleQual == 1)
		{
			texSampleQual = 0;
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Texture Sampling Quality: Nearest Neighbor");

			text->activateFade(1);

			hud::hud.push_back(text);

			if (mipMapQual == 0)
				setTextureSamplingQuality(GL_NEAREST);
			else if (mipMapQual == 1)
				setTextureSamplingQuality(GL_NEAREST_MIPMAP_NEAREST);
			else
				setTextureSamplingQuality(GL_NEAREST_MIPMAP_LINEAR);

		}
	}

	if (glfwGetKey(window, GLFW_KEY_F5) && player::input_cooldown <= 0){
		player::input_cooldown = 0.3f;

		if (mipMapQual == 0)
		{
			mipMapQual = 1;
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Mip Mapping Quality: Nearest Neighbor");

			text->activateFade(1);

			hud::hud.push_back(text);

			if (texSampleQual == 0)
				setTextureSamplingQuality(GL_NEAREST_MIPMAP_NEAREST);
			else
				setTextureSamplingQuality(GL_LINEAR_MIPMAP_NEAREST);
		}
		else if (mipMapQual == 1)
		{
			mipMapQual = 2;
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Mip Mapping Quality: Linear");

			text->activateFade(1);

			hud::hud.push_back(text);

			if (texSampleQual == 0)
				setTextureSamplingQuality(GL_NEAREST_MIPMAP_LINEAR);
			else
				setTextureSamplingQuality(GL_LINEAR_MIPMAP_LINEAR);
		}
		else if (mipMapQual == 2)
		{
			mipMapQual = 0;
			scene::Text *text = new scene::Text(glm::mat4(1), shaders::tex_shader, -5, 3);
			text->scale = glm::scale(glm::mat4(1), glm::vec3(0.02, 0.02, 0.02));

			text->setText("Mip Mapping Quality: Off");

			text->activateFade(1);

			hud::hud.push_back(text);

			if (texSampleQual == 0)
				setTextureSamplingQuality(GL_NEAREST);
			else
				setTextureSamplingQuality(GL_LINEAR);

		}
	}

	if (glfwGetKey(window, GLFW_KEY_M) && player::input_cooldown <= 0){

		player::input_cooldown = 0.3f;
		//If the music is paused
		if (Mix_PausedMusic() == 1)
		{
			//Resume the music
			Mix_ResumeMusic();
		}
		//If the music is playing
		else
		{
			//Pause the music
			Mix_PauseMusic();
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F12) && player::input_cooldown <= 0){

		player::input_cooldown = 0.3f;
		// switch room
		switchRoom();
	}


	if (glfwGetKey(window, GLFW_KEY_E) && player::input_cooldown <= 0)
	{
		player::input_cooldown = 0.3f;

		if (simple_collision_detection(glm::translate(glm::mat4(1), player::currentRoom->goal_point), glm::scale(glm::mat4(1), glm::vec3(4, 2, 4)), glm::translate(glm::mat4(1), player::position)))
		{
			switchRoom();
			Mix_PlayChannel(-1, sounds::snd_level_complete, 0);
		}
	}


	if (glfwGetKey(window, GLFW_KEY_Q) && player::input_cooldown <= 0)
	{
		player::input_cooldown = 0.3f;
		if (player::currentRoom == safe_room &&
			simple_collision_detection(glm::translate(glm::mat4(1), glm::vec3(4, 1, 4)), glm::scale(glm::mat4(1), glm::vec3(5, 5, 5)), glm::translate(glm::mat4(1), player::position))
			&& player::coins_collected >= 5)
		{
			player::coins_collected -= 5;
			updateCoins();

			Mix_PlayChannel(-1, sounds::snd_gotCoin , 0);

			scene::Upgrade *temp_u;

			int temp_random = rand() % 7 + 2;
			switch (temp_random)
			{
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
			temp_u->translation = glm::translate(glm::mat4(1), glm::vec3(0, 2, 2));

			safe_room->upgrades.push_back(temp_u);
		}
	}

	// test: life toggling
	// TODO: delete this!
	/*if (glfwGetKey(window, GLFW_KEY_F6) && shoot_cooldown <= 0){

		shoot_cooldown = 0.3f;
		lives--;
		if (lives < 0)
			lives = 3;
	}
	*/

	// life drawing
	if (player::lives == 3)
	{
		life3->texture = life_texture;
		life2->texture = life_texture;
		life1->texture = life_texture;
	}
	else if (player::lives == 2)
	{
		life3->texture = life_lost_texture;
		life2->texture = life_texture;
		life1->texture = life_texture;
	}
	else if (player::lives == 1)
	{
		life3->texture = life_lost_texture;
		life2->texture = life_lost_texture;
		life1->texture = life_texture;
	}
	else
	{
		life3->texture = life_lost_texture;
		life2->texture = life_lost_texture;
		life1->texture = life_lost_texture;

		// (Game Over)
	}


	// movement

		if (using_joystick)			// apparently shouldnt be neccessary, but doesnt work without
		{
			joystick_axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes);
			joystick_buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons);

			if (joystick_axes[4] > 0.3f || joystick_axes[4] < -0.3f)
				player::horizontalAngle += deltaT * -joystick_axes[4] * 2.0f;

			if (joystick_axes[3] > 0.3f || joystick_axes[3] < -0.3f)
				player::verticalAngle += deltaT * -joystick_axes[3] * 2.0f;
		}

		// read mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		glfwSetCursorPos(window, double(width / 2), double(height / 2));

		//horizontalAngle += deltaT * float(width / 2 - xpos) * mouse_speed;
		//verticalAngle += deltaT * float(height / 2 - ypos) * mouse_speed;
		player::horizontalAngle += float(width / 2 - xpos) * player::mouse_speed * 0.005;
		player::verticalAngle += float(height / 2 - ypos) * player::mouse_speed * 0.005;

		if (player::verticalAngle > 3.14f / 2)
			player::verticalAngle = 3.14f / 2;

		if (player::verticalAngle < -3.14f / 2)
			player::verticalAngle = -3.14f / 2;

		direction = glm::vec3(
			cos(player::verticalAngle) * sin(player::horizontalAngle),
			sin(player::verticalAngle),
			cos(player::verticalAngle) * cos(player::horizontalAngle)
			);

		glm::vec3 move_direction(
			sin(player::horizontalAngle),
			0,
			cos(player::horizontalAngle)
			);

		glm::vec3 right(
			sin(player::horizontalAngle - 3.14f / 2.0f),
			0,
			cos(player::horizontalAngle - 3.14f / 2.0f)
			);

		up = glm::vec3(0, 1, 0);

	if (player::allow_move)
	{
		if (flying_debug)
		{
			// ascend
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
				player::position += up * float(deltaT) * player::speed;
			}

			// descend
			if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
				player::position -= up * float(deltaT) * player::speed;
			}

		}
		else
		{
			// jump
			if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || (using_joystick && joystick_buttons[0])) && player::jump_counter < player::max_jumps && player::jump_cooldown <= 0)
			{
				player::upwards_momentum = player::jump_momentum;
				player::jump_counter++;
				player::jump_cooldown = 0.5f;	// 0.5 sec jump cooldown (testing if appropriate)
				//Mix_PlayChannel(-1, s, 0);
			}

			if (player::upwards_momentum > 0)		// move upward
			{
				if (player::position.y >= player::room_height - 1)
					player::upwards_momentum = 0;
				else
				{
					player::position += up * float(deltaT) * player::upwards_momentum * 3.0f;
					player::upwards_momentum -= deltaT;
				}
				//std::cout << "moving up, u momentum: " << upwards_momentum << "\n";
			}
			else if (player::position.y > 1)			// move downward
			{
				// collision detection

				player::upwards_momentum -= deltaT * 3;
				glm::vec3 new_pos = player::position + up * float(deltaT) * player::upwards_momentum * 3.0f;

				for (int i = 0; i < player::currentRoom->cubes.size() && player::upwards_momentum < 0; i++)
				{
					if (simple_collision_detection_for_falling(player::currentRoom->cubes[i]->translation, player::currentRoom->cubes[i]->scale, player::position))
					{
						if (!simple_collision_detection_for_falling(player::currentRoom->cubes[i]->translation, player::currentRoom->cubes[i]->scale, new_pos))
						{
							player::upwards_momentum = 0;
							player::jump_counter = 0;
						}
					}
				}

				player::position += up * float(deltaT) * player::upwards_momentum * 3.0f;
				//std::cout << "moving down, u momentum: " << upwards_momentum << "\n";
				
				if (player::position.y <= 1)
				{
					player::position.y = 1;
					player::upwards_momentum = 0;
					player::jump_counter = 0;
				}
			}

		}


		// Move forward
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || (using_joystick && joystick_axes[1] < -0.5f)){
			player::position += move_direction * float(deltaT) * player::speed;

			//std::cout << position.x << ", " << position.y << ", " << position.z << "\n";

			if (!flying_debug && abs(player::position.x) >= player::currentRoom->room_width / 2 - 1)
				player::position.x -= move_direction.x * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.z) >= player::currentRoom->room_length / 2 - 1)
				player::position.z -= move_direction.z * float(deltaT) * player::speed;
		}
		// Move backward
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || (using_joystick && joystick_axes[1] > 0.5f)){
			player::position -= move_direction * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.x) >= player::currentRoom->room_width / 2 - 1)
				player::position.x += move_direction.x * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.z) >= player::currentRoom->room_length / 2 - 1)
				player::position.z += move_direction.z * float(deltaT) * player::speed;
		}
		// Strafe right
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || (using_joystick && joystick_axes[0] > 0.5f)){
			player::position += right * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.x) >= player::currentRoom->room_width / 2 - 1)
				player::position.x -= right.x * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.z) >= player::currentRoom->room_length / 2 - 1)
				player::position.z -= right.z * float(deltaT) * player::speed;
		}
		// Strafe left
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || (using_joystick && joystick_axes[0] < -0.5f)){
			player::position -= right * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.x) >= player::currentRoom->room_width / 2 - 1)
				player::position.x += right.x * float(deltaT) * player::speed;

			if (!flying_debug && abs(player::position.z) >= player::currentRoom->room_length / 2 - 1)
				player::position.z += right.z * float(deltaT) * player::speed;
		}

	}
		
		// SHOOT BULLETS!
		if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS || (using_joystick && (joystick_buttons[2] || joystick_axes[2] < -0.5f))) && player::shoot_cooldown == 0)
		{
			// create new bullet, using bullet shader, facing the direction the player is facing, moving at 2 units per sec and lasting 10 seconds, accelerating at 1 unit per second
			scene::Bullet *new_cube = new scene::Bullet(glm::mat4(1.0f), shaders::shader2, direction, 10 + player::bullet_speed_plus, 10, player::bullet_acceleration_plus);
			new_cube->materialColor = glm::vec4(0.6, 0.3, 0.1, 1);
			new_cube->scale = glm::scale(new_cube->scale, glm::vec3(0.1f, 0.1f, 0.1f));
			new_cube->translation = glm::translate(new_cube->translation, player::position + direction);

			if (player::currentRoom != nullptr)
				player::currentRoom->bullets.push_back(new_cube);

			player::shoot_cooldown = player::shoot_cooldown_value;	// cooldown in seconds
		}


		// SHOOT HOOKSHOT!
		if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || (using_joystick && (joystick_buttons[3] || joystick_axes[2] > 0.5f))) && player::hookshot_cooldown <= 0)
		{
			//std::cout << "shooting hookshot" << std::endl;
			player::hookshot_cooldown = 1;
			player::hookshot_head = new scene::Bullet(glm::mat4(1.0f), shaders::shader2, direction, 50, 0.2f + 0.1f * player::hookshot_length, 0);
			player::hookshot_head->scale = glm::scale(player::hookshot_head->scale, glm::vec3(0.2f, 0.2f, 0.2f));
			player::hookshot_head->translation = glm::translate(glm::mat4(1), player::position - right * 0.3f + glm::vec3(0, -0.2f, 0));
			player::hookshot_head->materialColor = glm::vec4(0.7, 0.7, 0.7, 1);

			player::hookshot_body->translation = glm::translate(glm::mat4(1), player::position - right * 0.3f + glm::vec3(0, -0.2f, 0));
			player::hookshot_body->rotation = glm::rotate(glm::mat4(1.0f), player::horizontalAngle - 0.01f, glm::vec3(0, 1, 0));
			player::hookshot_body->rotation = glm::rotate(player::hookshot_body->rotation, -player::verticalAngle, glm::vec3(1, 0, 0));
			player::hookshot_body->scale = glm::scale(glm::mat4(1), glm::vec3(0.03, 0.03, (0.2f + 0.1f * player::hookshot_length) * 100));

			player::allow_move = false;
		}

		if (player::hookshot_head != nullptr)
		{
			if (player::hooking)	// pull player toward hookshot head
			{
				player::position += player::hookshot_head->direction * deltaT * 30.0f;

				if (simple_collision_detection(glm::translate(glm::mat4(1), player::position), glm::scale(glm::mat4(1), glm::vec3(2, 2, 2)), player::hookshot_head->modelMatrix))
				{
					// hooking complete
					player::hooking = false;
					player::allow_move = true;

					delete player::hookshot_head;
					player::hookshot_head = nullptr;
				}

				if (!flying_debug && abs(player::position.x) >= player::currentRoom->room_width / 2 - 1)
					player::position.x -= player::hookshot_head->direction.x * deltaT * 30.0f;

				if (!flying_debug && abs(player::position.z) >= player::currentRoom->room_length / 2 - 1)
					player::position.z -= player::hookshot_head->direction.z * deltaT * 30.0f;
			}
			else					// move hookshot head
			{
				player::hookshot_head->update(deltaT);
				player::hookshot_body->update(deltaT);

				if (player::hookshot_head->time_to_live <= 0)
				{
					delete player::hookshot_head;
					player::hookshot_head = nullptr;
					player::allow_move = true;
				}
				else
				{
					// test for collision

					for (int i = 0; i < player::currentRoom->cubes.size(); i++)
					{
						if (player::currentRoom->cubes[i] != player::currentRoom->floorCube &&
							player::currentRoom->cubes[i] != player::currentRoom->ceilingCube &&
							player::currentRoom->cubes[i] != player::currentRoom->wallCube1 &&
							player::currentRoom->cubes[i] != player::currentRoom->wallCube2 &&
							player::currentRoom->cubes[i] != player::currentRoom->wallCube3 &&
							player::currentRoom->cubes[i] != player::currentRoom->wallCube4 &&
							simple_collision_detection(player::currentRoom->cubes[i]->translation, player::currentRoom->cubes[i]->scale, player::hookshot_head->modelMatrix))
						{
							player::hooking = true;
							//std::cout << "HOOK SUCCESSFUL!" << std::endl;
						}
					}

					if (player::hooking == false)
					{
						// hook onto enemies
						for (int i = 0; i < player::currentRoom->enemies.size(); i++)
						{
							if (simple_collision_detection(player::currentRoom->enemies[i]->translation, player::currentRoom->enemies[i]->scale, player::hookshot_head->modelMatrix))
							{
								player::hooking = true;
							}
						}
					}
				}
			}
		}

		view = glm::lookAt(player::position, player::position + direction, up);
		frustum.setCamDef(player::position, player::position + direction, up);

		//std::cout << "x: " << player::position.x << ", y: " << player::position.y << ", z: " << player::position.z << "\n";
		// generate vp matrix
		view_proj = proj * view;

		// upload to all shaders
		
		upload_vp(view_proj);

		shaders::tex_shader->useShader();
		auto view_proj_location = glGetUniformLocation(shaders::tex_shader->programHandle, "view_proj");
		glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(view_proj));
		
		ppShader->useShader();
		view_proj_location = glGetUniformLocation(ppShader->programHandle, "view_proj");
		glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(view_proj));

		thresholdShader->useShader();
		view_proj_location = glGetUniformLocation(thresholdShader->programHandle, "view_proj");
		glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(view_proj));

		blurShader->useShader();
		view_proj_location = glGetUniformLocation(blurShader->programHandle, "view_proj");
		glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(view_proj));

		shaders::tex_shader2->useShader();
		auto camera_position = glGetUniformLocation(shaders::tex_shader2->programHandle, "camera_position");
		glUniform3fv(camera_position, 1, glm::value_ptr(player::position));

		shaders::tex3dshader->useShader();
		camera_position = glGetUniformLocation(shaders::tex3dshader->programHandle, "camera_position");
		glUniform3fv(camera_position, 1, glm::value_ptr(player::position));

		armcannon->translation = glm::translate(glm::mat4(1.0f), player::position + direction *0.5f + right *0.5f + glm::vec3(0, -0.2f, 0));

		armcannon->rotation = glm::rotate(glm::mat4(1.0f), player::horizontalAngle, glm::vec3(0, 1, 0));
		armcannon->rotation = glm::rotate(armcannon->rotation, -player::verticalAngle, glm::vec3(1, 0, 0));


		// reduce cooldowns
		if (player::hookshot_cooldown > 0)
		{
			player::hookshot_cooldown -= deltaT;
			if (player::hookshot_cooldown < 0)
				player::hookshot_cooldown = 0;
		}

		if (player::shoot_cooldown > 0)
		{
			player::shoot_cooldown -= deltaT;
			if (player::shoot_cooldown < 0)
				player::shoot_cooldown = 0;
		}

		if (player::jump_cooldown > 0)
		{
			player::jump_cooldown -= deltaT;
			if (player::jump_cooldown < 0)
				player::jump_cooldown = 0;
		}

		if (player::input_cooldown > 0)
		{
			player::input_cooldown -= deltaT;
			if (player::input_cooldown < 0)
				player::input_cooldown = 0;
		}

		// update 2d stuff

		for (int i = 0; i < hud::hud.size(); i++)
		{
			hud::hud[i]->translation = glm::translate(glm::mat4(1.0f), player::position + direction * 0.2f);

			hud::hud[i]->rotation = glm::rotate(glm::mat4(1.0f), player::horizontalAngle, glm::vec3(0, 1, 0));
			hud::hud[i]->rotation = glm::rotate(hud::hud[i]->rotation, -player::verticalAngle, glm::vec3(1, 0, 0));

			hud::hud[i]->update(deltaT);
		}

		pressE->translation = glm::translate(glm::mat4(1.0f), player::position + direction * 0.2f);

		pressE->rotation = glm::rotate(glm::mat4(1.0f), player::horizontalAngle, glm::vec3(0, 1, 0));
		pressE->rotation = glm::rotate(pressE->rotation, -player::verticalAngle, glm::vec3(1, 0, 0));

		pressE->update(deltaT);

		pressQ->translation = glm::translate(glm::mat4(1.0f), player::position + direction * 0.2f);

		pressQ->rotation = glm::rotate(glm::mat4(1.0f), player::horizontalAngle, glm::vec3(0, 1, 0));
		pressQ->rotation = glm::rotate(pressQ->rotation, -player::verticalAngle, glm::vec3(1, 0, 0));

		pressQ->update(deltaT);
		
		// update objects
		//obj->rotation = glm::rotate(obj->rotation, float(deltaT), glm::vec3(1, 1, 1));

		armcannon->update(deltaT);

		// update room
		if (player::currentRoom != nullptr)
		{
			player::currentRoom->update(deltaT);
		}

		//model->update(deltaT);
}

void createShadowMaps()
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.6f, 0.0f);

	// disable backface culling
	glDisable(GL_CULL_FACE);

	frustum.setCamInternals(135.0f, 1.0f, 1.0f, player::currentRoom->room_length + 1.0f);

	shaders::shader2->useShader();
	auto sloc = glGetUniformLocation(shaders::shader2->programHandle, "simpleDraw");
	//std::cout << "tex loc " << i << " " << texLoc << std::endl;
	glUniform1i(sloc, 1);

	//auto light_proj = glm::ortho<float>(-3, 3, -3, 3, 1, 50);
	auto light_proj = glm::perspective(glm::radians(135.0f), 1.0f, 0.1f, player::currentRoom->room_length + 1.0f);

	glViewport(0, 0, height / shadowMappingRectionFactor, height / shadowMappingRectionFactor);

	for (int i = 0; i < player::currentRoom->lights.size(); i++)
	{
		// look at center of room
		auto light_view = glm::lookAt(player::currentRoom->lights[i].position, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

		frustum.setCamDef(player::currentRoom->lights[i].position, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

		glBindFramebuffer(GL_FRAMEBUFFER, sm_fb[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_2D, sm_depth[i]);

		// generate view_proj
		//shaders::zBufferShader->useShader();
		auto light_view_proj = light_proj * light_view;

		//auto view_proj_location = glGetUniformLocation(shaders::shader2->programHandle, "view_proj");
		//glUniformMatrix4fv(view_proj_location, 1, GL_FALSE, glm::value_ptr(light_view_proj));
		upload_vp(light_view_proj);

		player::currentRoom->draw(&frustum);// _zBuffer(true);

		std::ostringstream pos;
		pos << "DepthVP[" << i << "]";

		shaders::shader2->useShader();
		GLuint depthVPpos = glGetUniformLocation(shaders::shader2->programHandle, pos.str().c_str());
		glUniformMatrix4fv(depthVPpos, 1, GL_FALSE, glm::value_ptr(light_view_proj));

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, sm_depth[i]);
	}

	glViewport(0, 0, width, height);

	shaders::shader2->useShader();
	sloc = glGetUniformLocation(shaders::shader2->programHandle, "simpleDraw");
	//std::cout << "tex loc " << i << " " << texLoc << std::endl;
	glUniform1i(sloc, 0);
	upload_vp(view_proj);

	glDisable(GL_POLYGON_OFFSET_FILL);

	// enable backface culling
	glEnable(GL_CULL_FACE);

	frustum.setCamInternals(60.0f, width / (float)height, 0.1f, 100.0f); 
	frustum.setCamDef(player::position, player::position + direction, up);

	for (int i = 0; i < player::currentRoom->lights.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, sm_depth[i]);
	}
}

void draw()
{
	if (use_shadowmapping)
		createShadowMaps();

	glActiveTexture(GL_TEXTURE0);

	// draw into buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw duck
	
	//model->draw();
	
	if (wireFrame)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (transparency)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if (player::currentRoom != nullptr)
		player::currentRoom->draw(&frustum);

	if (player::currentRoom == safe_room)
		duck->draw();

	if (player::hookshot_head != nullptr)
	{

		// disable backface culling
		glDisable(GL_CULL_FACE);

		player::hookshot_body->draw();
		
		// enable backface culling
		glEnable(GL_CULL_FACE);
		
		if (!player::hooking)
		{
			// disable depth buffer
			//glDisable(GL_DEPTH_TEST);

			player::hookshot_head->draw();

			// enable depth buffer
			//glEnable(GL_DEPTH_TEST);
		}
	}

	// disable depth testing (z buffering)
	glDisable(GL_DEPTH_TEST);
	armcannon->draw();

	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindFramebuffer(GL_FRAMEBUFFER, bloomFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	screen->shader = ppShader;
	screen->draw();


	// draw fbo onto screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, bloomFbo_texture);
	screen->draw();


	// draw with threshold filter
	glBindFramebuffer(GL_FRAMEBUFFER, smallFbo);
	glViewport(0, 0, width / bloomReductionFactor, height / bloomReductionFactor);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	
	screen->shader = thresholdShader;
	screen->draw();
	
	// draw with blur onto screen (= bloom)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, smallFbo_texture);

	screen->shader = blurShader;
	screen->draw();

	if (transparency)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	// draw hud
	for (int i = 0; i < hud::hud.size(); i++)
	{
		if (hud::hud[i] != screen)
			hud::hud[i]->draw();
	}

	// draw level advance message
	if (simple_collision_detection(glm::translate(glm::mat4(1), player::currentRoom->goal_point), glm::scale(glm::mat4(1), glm::vec3(4, 2, 4)), glm::translate(glm::mat4(1), player::position)))
		pressE->draw();

	// draw buy coins message
	if (player::currentRoom == safe_room &&
			simple_collision_detection(glm::translate(glm::mat4(1), glm::vec3(4, 1, 4)), glm::scale(glm::mat4(1), glm::vec3(5, 5,5 )), glm::translate(glm::mat4(1), player::position)))
		pressQ->draw();

	glEnable(GL_BLEND);
	
	// draw shadowmaps on screen
	if (draw_shadowmaps)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screen->shader = ppShader;
		ppShader->useShader();

		for (int i = 0; i < player::currentRoom->lights.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, sm_depth[i]);

			glViewport(110 * i, 0, 100, 100);
			screen->draw();
		}
	}
		
	// enable depth testing (z buffering)
	glEnable(GL_DEPTH_TEST);

}

void cleanup()
{
	glfwTerminate();
	
	delete danbo_texture;
	danbo_texture = nullptr;

	delete life_texture;
	life_texture = nullptr;

	delete crosshair_texture;
	crosshair_texture = nullptr;

	delete scene_textures::texture;
	scene_textures::texture = nullptr;

	delete scene_textures::texture2;
	scene_textures::texture2 = nullptr;

	delete scene_textures::coinTexture;
	scene_textures::coinTexture = nullptr;

	delete scene_textures::plus_hook_length;
	scene_textures::plus_hook_length = nullptr;

	delete scene_textures::healthpack;
	scene_textures::healthpack = nullptr;

	delete scene_textures::plus_bullet_accel;
	scene_textures::plus_bullet_accel = nullptr;

	delete scene_textures::plus_bullet_speed;
	scene_textures::plus_bullet_speed = nullptr;

	delete scene_textures::plus_jump_momentum;
	scene_textures::plus_jump_momentum = nullptr;

	delete scene_textures::plus_max_jumps;
	scene_textures::plus_jump_momentum = nullptr;

	delete scene_textures::plus_shoot_rate;
	scene_textures::plus_shoot_rate = nullptr;

	delete scene_textures::plus_speed;
	scene_textures::plus_speed = nullptr;
	
	delete ppShader;
	ppShader = nullptr;
	
	delete thresholdShader;
	thresholdShader = nullptr;
	
	delete blurShader;
	blurShader = nullptr;

	delete shaders::zBufferShader;
	shaders::zBufferShader = nullptr;

	delete shaders::shader;
	shaders::shader = nullptr;

	delete shaders::shader2;
	shaders::shader2 = nullptr;

	delete shaders::tex_shader;
	shaders::tex_shader = nullptr;

	delete shaders::tex_shader2;
	shaders::tex_shader2 = nullptr;
	
	delete armcannon;
	armcannon = nullptr;

	delete player::hookshot_body;
	player::hookshot_body = nullptr;

	delete player::hookshot_head;
	player::hookshot_head = nullptr;
	
	for (int i = 0; i < hud::hud.size(); i++)
	{
		delete hud::hud[i];
		hud::hud[i] = nullptr;
	}

	delete pressE;
	pressE = nullptr;

	delete pressQ;
	pressQ = nullptr;

	delete main_room;
	main_room = nullptr;

	delete safe_room;
	safe_room = nullptr;

	//Free the sound effects
	Mix_FreeChunk(sounds::snd_game_over);
	sounds::snd_game_over = NULL;
	Mix_FreeChunk(sounds::snd_gotCoin);
	sounds::snd_gotCoin = NULL;
	Mix_FreeChunk(sounds::snd_gotItem);
	sounds::snd_gotItem = NULL;
	Mix_FreeChunk(sounds::snd_level_complete);
	sounds::snd_level_complete = NULL;
	//Free the music
	Mix_FreeMusic(sounds::m_Music);
	sounds::m_Music = NULL;

	//Quit SDL subsystems
	Mix_Quit();
	SDL_Quit();

	glDeleteRenderbuffers(1, &rbo_depth);
	glDeleteTextures(1, &fbo_texture);
	glDeleteFramebuffers(1, &fbo);

	glDeleteRenderbuffers(1, &bloomRbo_depth);
	glDeleteTextures(1, &bloomFbo_texture);
	glDeleteFramebuffers(1, &bloomFbo);

	glDeleteRenderbuffers(1, &smallRbo_depth);
	glDeleteTextures(1, &smallFbo_texture);
	glDeleteFramebuffers(1, &smallFbo);

	std::cout << "\n\n=========\n Stats:\n=========\n"
		<< "\nLevel reached: " << player::level
		<< "\nCoins collected: " << player::coins_collected
		<< "\nLives: " << player::lives
		<< "\nMovement speed: " << player::speed
		<< "\nMaximum jumps: " << player::max_jumps
		<< "\nShooting cooldown: " << player::shoot_cooldown_value
		<< "\nJump height: " << player::jump_momentum
		<< "\nBullet Speed: " << (10 + player::bullet_speed_plus)
		<< "\nBullet Acceleration: " << player::bullet_acceleration_plus
		<< "\nHook length: " << player::hookshot_length
		<< "\n\n\n";

	system("pause");
}


static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		typeString = "Portability";
		break;
	}
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		break;
	}
	}

	stringStream << "OpenGL Error: " << msg;
	stringStream << " [Source = " << sourceString;
	stringStream << ", Type = " << typeString;
	stringStream << ", Severity = " << severityString;
	stringStream << ", ID = " << id << "]";

	return stringStream.str();
}