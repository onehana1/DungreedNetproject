#pragma once
#ifndef _player
#define _player
#include "Character.h"
#include "Uncopyable.h"
#include "InstantDCSet.h"
#include "Weapon.h"
#include "HitScan.h"
#include "Protocol.h"
#include <cstring>

extern HDC buf_dc;
extern RECT client;

const double PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE = 10.0;
const double PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE = 5.0;

class Crosshair;
class Weapon;

class Player : private Uncopyable, public Character
{
private:
	char name[20];
	in_addr ip;

	short server_state;

	//HANDLE thread;

	double dash_power = 0;	// dash_power > 0 이면 dash중인 상태, dash_power < 0 이면 다음 dash 가능 시간까지 대기중, dash_power == 0이면 dash 가능 상태
	double dash_radian = 0;

	double atk_radian;

	int walk_cnt = 0;

	bool is_doing_missile_attack;

	void KeyProc(const Dungeon* dungeon, MissileManager* missile_manager);
	void DashProc(float radian, const Dungeon* dungeon, const int px);
	void AttackProc(Weapon* weapon, MissileManager* missile_manager);

	void SC_KeyProc(const Dungeon* dungeon, PLAYER_KEYBOARD key, POINT* Ppos);
	void SC_DashProc(float radian, const Dungeon* dungeon, int *px);
	void SC_AttackProc(Weapon* weapon, MissileManager* missile_manager, PLAYER_KEYBOARD key, PLAYER_MOUSE mouse);
public:
	Player() = default;
	Player(const Dungeon* dungeon) :
		Character(1234567, dungeon->camera_x_half_range / PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE,
			dungeon->camera_x_half_range / PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE,
			dungeon->left_start_pos, State::DOWN, TRUE,
			dungeon->camera_x_half_range / 60.0f, dungeon->camera_y_half_range / 32.0f, 200, 100, 50){}

	SOCKET sock;

	void PlaceWithDungeonLeft(const Dungeon* dungeon);
	void PlaceWithDungeonRight(const Dungeon* dungeon);

	void Init(const Dungeon* dungeon  );

	void Update(const Dungeon* dungeon, Weapon* weapon, MissileManager* missile_manager );
	void SC_Update(const Dungeon* dungeon, PLAYER_MOUSE mouse, PLAYER_KEYBOARD key, POINT* Ppos); //
	void SC_Update2(const Dungeon* dungeon, Weapon* weapon, MissileManager* missile_manager, PLAYER_KEYBOARD key, PLAYER_MOUSE mouse); //
	PLAYER_INFO_MANAGER info;

	friend class Weapon;
	friend class MonsterAI;
	POINT mouse;

	void SetState(short p_state) { server_state = p_state; }
	short GetState() { return server_state; }

	void SetName(char* p_name) { strcpy(name, p_name); }
	char* GetName() { return name; }
	bool GetMisile() { return is_doing_missile_attack; }

	void SetIp(in_addr p_ip) { ip = p_ip; }
	in_addr GetIp() { return ip; }
};
#endif

