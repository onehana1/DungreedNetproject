#pragma once
#ifndef _weapon
#define _weapon
#include <windows.h>
#include <math.h>
#include "FileUtility.h"
#include "Player.h"
#include "Scene.h"
#include <string>

class Player;
class Scene;

class Weapon
{
private:
	const Image* image = NULL;
	const Image* start_image = NULL;

	POINT pos;
	int width;
	int height;
	float angle;
	BOOL looking_direction;

	int ATTACT_TIME;
public:  
	Weapon(const Dungeon* dungeon, const Player* player);
	~Weapon();
	void Init(const Dungeon* dungeon, const Player* player, const POINT mouse);
	void Update(const Player* player, const POINT mouse );
	void Render(HDC scene_dc, const RECT& bit_rect);
	void StartAttack();
	bool IsAttackFinished();
};
#endif