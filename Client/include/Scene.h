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

extern HWND h_wnd;
extern void DrawBuffer(HDC instant_dc, const RECT& rect);

class Scene : private Uncopyable
{
private:
public:
	Scene();
	~Scene();

	virtual void Render() const;
	virtual void Update();
	virtual int ChangeScene();
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
	virtual void Update();

	virtual int ChangeScene();
};

class StartScene : public Scene
{
private:
	Crosshair* crosshair;
	Image* image;
	

	int update_cnt = 0;

public:
	StartScene();
	~StartScene();

	virtual void Render() const;
	virtual void Update();

	virtual int ChangeScene();
};

class LobbyScene : public Scene
{
private:
	Crosshair* crosshair;
	Image* image;

	int update_cnt = 0;

public:
	LobbyScene();
	~LobbyScene();

	virtual void Render() const;
	virtual void Update();

	virtual int ChangeScene();
};
#endif