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
	const Image* image;
	const Image* start_image;

	POINT pos;
	int width;
	int height;
	float angle;
	BOOL looking_direction;

	int ATTACT_TIME;
public:  
	Weapon(const Scene* scene, const Player* player,const POINT mouse);
	~Weapon();
	void Init(const Scene* scene, const Player* player, const POINT mouse);
	void Update(const Player* player, const POINT mouse );
	void Render(HDC scene_dc, const RECT& bit_rect);
	void StartAttack();
	bool IsAttackFinished();
};
#endif