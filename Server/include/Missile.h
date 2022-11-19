#pragma once
#ifndef _missile
#define _missile
#include "FileUtility.h"
#include "Windows.h"
#include "Character.h"
#include <math.h>
#include <vector>
#include <string>
#include <cstring>
#define pi 3.141592

extern bool MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt);
extern bool CanGoToPos(const HDC terrain_dc, const POINT pos);

class Missile {
private:
	const Image* image;
	const Image* start_image;

	bool is_animation_load_requested = false;
	std::string animation_name;

	POINT old_pos;
	POINT pos;
	int width;
	int height;
	float radian;
	int speed;
	int range;
	BOOL looking_direction;
	int pierce;					// ฐüล๋ทย
	int atk;

	std::vector<const Character*> attack_victims;
	RECT atk_rect;
	char atk_sound_name[FILE_NAME_LEN];
	float atk_sound_volume;

	bool IsOutOfRange() const;


	void AddAttackVictim(const Character* victim);
	bool HasAlreadyAttacked(const Character* victim) const;
public:
	const Character* host;

	Missile(Character* host, const POINT pos, const int width, const int height, const float radian,
		const int speed, const int range, const BOOL looking_direction, const int pierce, const int atk )
		: host{ host }, pos{ pos }, old_pos{ pos }, width{ width }, height{ height }, radian{ radian },
		speed{ speed }, range{ range }, looking_direction{ looking_direction }, pierce{ pierce }, atk{ atk }{}

	~Missile()
	{
		delete start_image;
	}

	void Update();
//	void Render(HDC scene_dc, const RECT& bit_rect) const;

	bool IsOut_Left(const Dungeon* dungeon) const;
	bool IsOut_Right(const Dungeon* dungeon) const;

	friend class MissileManager;
	friend class HitScanner;
};

class MissileManager {
private:
public:
	std::vector<Missile*>  missiles;

	void Init();
	void Render(HDC scene_dc, const RECT& bit_rect) const;
	void Update(const Dungeon* dungeon);
	void Insert(Missile* given_missile);
	void Delete(Missile* given_missile);
};

#endif