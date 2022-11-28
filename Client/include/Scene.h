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

	


	char* name;
};

class PlayScene : public Scene
{
private:
	Dungeon* dungeon;
	Player* player;
	Weapon* weapon;
	Camera* camera;
	Crosshair* crosshair;
	AnimationManager* animation_manager;
	MonsterManager* monster_manager;
	EffectManager* effect_manager;
	SoundManager* sound_manager;
	MissileManager* missile_manager;

	int update_cnt = 0;

	void GoNextDungeon();
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

	MonsterManager* GetMonsterManager(){return monster_manager;};
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
#endif