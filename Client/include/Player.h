#pragma once
#ifndef _player
#define _player
#include "Character.h"
#include "Uncopyable.h"
#include "InstantDCSet.h"
#include "Crosshair.h"
#include "Sound.h"
#include "Weapon.h"
#include "HitScan.h"
#include <cstring>
#include "Effect.h"

extern HDC buf_dc;
extern RECT client;

const double PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE = 10.0;
const double PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE = 5.0;

class Crosshair;
class Weapon;

class Player : private Uncopyable, public Character
{
private:
	PLAYER_INFO info;
	bool have_to_update = false;

	short id[3];
	char ip[22];
	char name[20];
	short server_state;

	int kill_monster;

	double dash_power = 0;	// dash_power > 0 이면 dash중인 상태, dash_power < 0 이면 다음 dash 가능 시간까지 대기중, dash_power == 0이면 dash 가능 상태
	double dash_radian = 0;

	double atk_radian;

	int walk_cnt = 0;

	bool is_doing_missile_attack;

	void KeyProc(const Dungeon* dungeon, MissileManager* missile_manager, SoundManager* sound_manager);
	void DashProc(float radian, const Dungeon* dungeon, const int px, SoundManager* sound_manager);
	void AttackProc(Weapon* weapon, MissileManager* missile_manager, AnimationManager* animation_manager, SoundManager* sound_manager);

	void MatchStateAndAnimation(AnimationManager* animation_manager, SoundManager* sound_manager, EffectManager* effect_manager);

public:
	Player() = default;
	Player(int x_half,int y_half, POINT start, AnimationManager* animation_manager) :
		Character(1234567, x_half / PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE,
			x_half / PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE,
			start, State::DOWN, TRUE,
			x_half / 60.0f, y_half / 32.0f, "player_stand",
			L"animation/player_stand1.png", 200, 100, 50, animation_manager)
	{
		animation.LoadAnimation(animation_manager, "player_stand");
		animation.Play();
		strcpy_s(atk_sound_name, "sound\\Slash2.ogg");
		atk_sound_volume = 0.4f;
	}

	Player(const Dungeon* dungeon, AnimationManager* animation_manager) :
		Character(1234567, dungeon->camera_x_half_range / PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE,
			dungeon->camera_x_half_range / PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE,
			dungeon->left_start_pos, State::DOWN, TRUE,
			dungeon->camera_x_half_range / 60.0f, dungeon->camera_y_half_range / 32.0f, "player_stand",
			L"animation/player_stand1.png", 200, 100, 50, animation_manager)
	{
		animation.LoadAnimation(animation_manager, "player_stand");
		animation.Play();
		strcpy_s(atk_sound_name, "sound\\Slash2.ogg");
		atk_sound_volume = 0.4f;
	}

	void PlaceWithDungeonLeft(const Dungeon* dungeon);
	void PlaceWithDungeonRight(const Dungeon* dungeon);

	void Init(const Dungeon* dungeon, AnimationManager* animation_manager);

	void Update(const Dungeon* dungeon, Weapon* weapon, MissileManager* missile_manager, AnimationManager* animation_manager, SoundManager* sound_manager, EffectManager* effect_manager);

	void SetState(short p_state) { server_state = p_state; }
	short GetState() { return server_state; }
	
	void SetName(char* p_name) { strcpy(name, p_name); }
	char* GetName() { return name; }

	void SetIp(char* p_ip) { strcpy(ip, p_ip); }
	char* GetIp() { return ip; }


	void SetKillMonster(int p_killmonster) { kill_monster = p_killmonster; }
	int GetKillMonster() { return kill_monster; }

	
	PLAYER_INFO GetInfo() { return info; }

	void SetPlayerInfo(PLAYER_INFO p_info);

	void ChangeStateToMoving();
	void ChangeStateToStanding();
	void MatchStateAndAnimation(AnimationManager* animation_manager, EffectManager* effect_manager);

	friend class Camera;
	friend class Weapon;
	friend class MonsterAI;
};
#endif

