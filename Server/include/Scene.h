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
	Player** TestPlayer;   // �Ẹ���� �÷��̾� 
	PLAYER_INFO_MANAGER* SC_INFO;  //���� ���� ���� ( Server -> client) 
	Weapon* weapon;

	MonsterManager* monster_manager;
	MissileManager* missile_manager;

	int update_cnt = 0;
	int Play_count = 0; // �÷��� ���� ��� �� 

	void GoNextDungeon();
	void GoPrevDungeon();
	void ChangeDungeon(const int dungeon_id);
	HRESULT Init();

	HitScanner HitScan;
	void HitUpdate();
	void DungeonChangeProc();

	///////////////////////////////////////////////////////
	void InputUpdate(CS_PLAYER_INPUT_INFO_PACKET INFO);
	void UpdateInfo(int num, Player* player);
	
public:
	void SetPlayer(int num) { // ���� �ο���ŭ �÷��� ���� 
		TestPlayer = new Player*[num];
		Play_count = num; 
		SC_INFO = new PLAYER_INFO_MANAGER[num];
	}

	PLAYER_INFO_MANAGER Player_Info[PLAYER_NUM];
	CS_PLAYER_INPUT_INFO_PACKET CS_Player_Info[PLAYER_NUM]; // �������� ������ ��ü. 

public:
	Scene();
	Scene(const int dungeon_id);
	~Scene();

    POINT DungeonSize{ 100,100 };

	//void Render() const;
	void Update();
};



#endif