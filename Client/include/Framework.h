#pragma once
#ifndef _framework
#define _framework
#include <WS2tcpip.h>
#include "Scene.h"
#include "Common.h"

extern DWORD WINAPI RecvThread(LPVOID arg);

class Framework
{
private:

	int scene_id;

	int update_cnt = 0;

	SOCKET sock; 
	char nickname[20];
	int count_name = 0;

	
public:
	Scene* scene;
	PlayScene* play_scene;
	int StartNum = 1;
	int Checking = 0;
	int dungeonID = 0;
	Framework();
	~Framework();

	void Render() const;
	void Update();
	void ChangeScene(int);
	int GetSceneId() { return scene_id; }
	void SetNickname(WPARAM wparam);

	PlayScene* GetPlayScene(){ return play_scene; }
};
#endif