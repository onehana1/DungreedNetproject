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
	Player* player[3];
	
	
	Weapon* weapon[3];

	// 플레이어 3명이면 weapon도 3개 여야 함

	MonsterManager* monster_manager;
	MissileManager* missile_manager;

	int update_cnt = 0;
	int Play_count = 0; // 플레이 중인 사람 수 

	
	void GoPrevDungeon();
	void ChangeDungeon(const int dungeon_id);
	

	HitScanner HitScan;
	void HitUpdate();
	void DungeonChangeProc();

	///////////////////////////////////////////////////////
public:
	Player** TestPlayer;   // 써보려는 플레이어 
	PLAYER_INFO_MANAGER* SC_INFO;  //보낼 정보 정리 ( Server -> client) 이놈이 변경된 보낼 정보 가지고잇음
	void InputUpdate(PLAYER_INPUT_INFO INFO);
	void UpdateInfo(int num, Player* player);
	Dungeon* GetDungeon() { return dungeon; };
	void GoNextDungeon();

	
public:
	HRESULT Init();
	void SetPlayer(int num) { // 들어온 인원만큼 플레이 갱신 
		TestPlayer = new Player*[num];
		Play_count = num; 
		SC_INFO = new PLAYER_INFO_MANAGER[num];
	}
	bool Check_Dun_Change[3]{ false,false,false };
	PLAYER_INFO_MANAGER Player_Info[PLAYER_NUM];
	CS_PLAYER_INPUT_INFO_PACKET CS_Player_Info[PLAYER_NUM]; // 서버에서 받으면 대체. 

public:
	Scene();
	Scene(const int dungeon_id);
	~Scene();

    POINT DungeonSize{ 100,100 };

	//void Render() const;
	void Update();

	void Send();

	//void UpdatePlayerInfo(short id, PLAYER_INFO p_info);
};



#endif