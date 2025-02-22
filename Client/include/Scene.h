#pragma once
#ifndef _scene
#define _scene
#include "Dungeon.h"
#include "Player.h"
#include "Monster.h"
#include "Camera.h"
#include "Uncopyable.h"
#include "Weapon.h"
#include "Crosshair.h"
#include <map>
#include <vector>
#include "FileUtility.h"
#include "Animation.h"
#include "HitScan.h"
#include "Effect.h"
#include "Sound.h"
#include "Missile.h"
#include "Protocol.h"
#include "time.h"


extern std::vector<Player*> player_list;
extern int g_myid;
extern HWND h_wnd;
extern void DrawBuffer(HDC instant_dc, const RECT& rect);

class Scene : private Uncopyable
{
private:
public:
	Scene();
	~Scene();

	virtual void Render() const;
	virtual void Update(SOCKET socket, char* name);
	virtual int ChangeScene();

	SOCKET server_sock;
	SOCKET sock;
	SOCKET Psock;
	
	PLAYER_INFO_MANAGER Player_Info[3];

	char* name;
};

class PlayScene : public Scene
{
private:
	Dungeon* dungeon;
	Player* player[3];
	Weapon* weapon[3];
	Camera* camera;
	Crosshair* crosshair;
	AnimationManager* animation_manager;
	MonsterManager* monster_manager;
	EffectManager* effect_manager;
	SoundManager* sound_manager;
	MissileManager* missile_manager;

	int update_cnt = 0;


	void GoDungeonResult();

	
	void GoPrevDungeon();
	void ChangeDungeon(const int dungeon_id);
	HRESULT Init();

	HitScanner HitScan;
	void HitUpdate();
	void DungeonChangeProc();
	void LoadPlayerAniamtion();
	void LoadBattleSound();
	void LoadBattleEffect();

public:
	PlayScene();
	PlayScene(const int dungeon_id);
	~PlayScene();

	virtual void Render() const;
	virtual void Update(SOCKET socket, char* name);

	virtual int ChangeScene();
	void GoNextDungeon();
	Dungeon* GetDungeon() { return dungeon; };
	virtual void SetDungeon(int dungeonID) {
		try {
			ChangeDungeon(dungeonID);
			Init();
			for (int i = 0; i < PLAYER_NUM; ++i) {
				player[i]->PlaceWithDungeonLeft(dungeon);
			}
		}
		catch (const TCHAR* error_message) {
			MessageBox(h_wnd, error_message, L"Error", MB_OK);
		}
	};

	Player** GetPlayer() { return player; }
	MonsterManager* GetMonsterManager(){return monster_manager;};
	MissileManager* GetMissileManager() { return missile_manager; };
	AnimationManager* GetAnimationManager() { return animation_manager; };

	void UpdateInfo(Player* player);
	void SetPlayerInfo(PLAYER_INFO[PLAYER_NUM]);
	void DeleteMissile(int id);
	void UpdateMissile(MISSILE_INFO missile[MISSILE_NUM]);
	
};

class StartScene : public Scene
{
private:
	Crosshair* crosshair;
	Image* image;

	int update_cnt = 0;

	char* nickname;

public:
	StartScene(char*);
	~StartScene();

	virtual void Render() const;
	virtual void Update(SOCKET socket, char* name);

	virtual int ChangeScene();
};

class LobbyScene : public Scene
{
private:
	Crosshair* crosshair;
	Image* image;
	Image* start_button;
	AnimationManager* animation_manager;
	EffectManager* effect_manager;
	Player* player[PLAYER_NUM];

	int update_cnt = 0;

public:
	LobbyScene();
	LobbyScene(SOCKET sock, char* name);
	~LobbyScene();

	virtual void Render() const;
	virtual void Update(SOCKET socket, char* name);

	virtual int ChangeScene();
};



class InterimScene : public Scene
{
private:
	Crosshair* crosshair;
	Image* image;
	Image* start_button;
	AnimationManager* animation_manager;
	EffectManager* effect_manager;
	Player* player[PLAYER_NUM];

	int update_cnt = 0;

public:
	InterimScene();
	InterimScene(SOCKET sock, char* name);
	~InterimScene();

	virtual void Render() const;
	virtual void Update(SOCKET socket, char* name);

	virtual int ChangeScene();
};

#endif