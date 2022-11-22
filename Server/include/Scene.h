#pragma once
#ifndef _scene
#define _scene
#include "Dungeon.h"
#include "Player.h"
#include "Monster.h"
#include "Uncopyable.h"
#include "Weapon.h"
#include <map>
#include <vector>
#include "FileUtility.h"
#include "HitScan.h"
#include "Missile.h"
#include "Protocol.h"


extern HWND h_wnd;
extern void DrawBuffer(HDC instant_dc, const RECT& rect);

class Dungeon;
class Player;
class Weapon; 
class MonsterManager;
class MissileManager;
class HitScanner;
class Uncopyable;

class Scene : private Uncopyable
{
private:
	Dungeon* dungeon;
	Player* player;
	Weapon* weapon;

	MonsterManager* monster_manager;
	MissileManager* missile_manager;

	int update_cnt = 0;

	void GoNextDungeon();
	void GoPrevDungeon();
	void ChangeDungeon(const int dungeon_id);
	HRESULT Init();

	HitScanner HitScan;
	void HitUpdate();
	void DungeonChangeProc();

	///////////////////////////////////////////////////////
	void InputUpdate(CS_PLAYER_INPUT_INFO_PACKET INFO);

	PLAYER_INFO_MANAGER Player_Info[PLAYER_NUM];
	CS_PLAYER_INPUT_INFO_PACKET CS_Player_Info[PLAYER_NUM]; // 서버에서 받으면 대체. 

public:
	Scene();
	Scene(const int dungeon_id);
	~Scene();

    POINT DungeonSize{ 100,100 };

	//void Render() const;
	void Update();
};



#endif