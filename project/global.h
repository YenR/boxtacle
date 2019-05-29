#pragma once
#include "Texture.h"
#include "HUDObject.h"
#include "Bullet.h"
#include <vector>

#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

extern int objects_rendered;
extern int gameOver;

namespace hud
{
	extern std::vector<scene::HUDObject*> hud;
}

namespace shaders
{
	extern Shader* shader;			// damage shader
	extern Shader* shader2;			// material shader
	extern Shader* tex_shader;		// 2d texture shader
	extern Shader* tex_shader2;		// 3d texture shader

	extern Shader* tex3dshader;		// shader for obj models

	extern Shader* zBufferShader;	// shader for shadow mapping
}

namespace scene_textures
{
	extern Texture *texture, *texture2, *coinTexture,
		*healthpack, *plus_speed, *plus_max_jumps,
		*plus_shoot_rate, *plus_jump_momentum,
		*plus_bullet_speed, *plus_bullet_accel, *plus_hook_length;

	extern Texture *text_texture;
}


namespace player	// player stats/values/options
{
	extern int level;

	extern int room_width;							// dimensions of current room
	extern int room_height;
	extern int room_length;	

	extern std::vector<scene::Bullet*> *enemy_bullets;	// enemy bullets in current room

	extern glm::vec3 position;						// position in world

	extern float hookshot_length;					// length of the hookshot
	extern int lives;								// current lifes (hearts)
	extern float speed;								// movement speed
	
	extern float shoot_cooldown_value;				// actual value in seconds between shots
	extern int max_jumps;							// = 1 normal + x air jumps
	extern float jump_momentum;						// added momentum on jumps

	extern int coins_collected;						// number of coins collected

	extern float bullet_speed_plus;					// added bullet speed
	extern float bullet_acceleration_plus;			// added bullet acceleration
	extern float bullet_damage_plus;				// added bullet damage (onto enemies)
}

namespace sounds // sounds
{
	// Music initializing
	extern Mix_Music *m_Music;

	//The sound effects that will be used
	extern Mix_Chunk *snd_gotItem;
	extern Mix_Chunk *snd_gotCoin;
	extern Mix_Chunk *snd_game_over;
	extern Mix_Chunk *snd_level_complete;
}
/**
* Uses translation and scale of an object to create an axis aligned cube for collision detection
* Then tests if the center of the bullet is in said cube, returning true if thats the case.
*/
extern bool simple_collision_detection(glm::mat4 &translation, glm::mat4 &scale, glm::mat4 &bullet);

/**
* Used for upgrade collision, gives upgrades a 1x1 axis aligned cube as hitbox.
*/
extern bool simple_collision_detection_for_upgrades(glm::mat4 &translation, glm::vec3 &player_position);

extern void updateCoins();

extern glm::mat4 getViewMatrix();
extern glm::mat4 getProjMatrix();

struct Light {
	glm::vec3 position;
	glm::vec3 intensities; //a.k.a. the color of the light
};