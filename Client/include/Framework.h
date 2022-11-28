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
	Scene* scene;
	PlayScene* play_scene;
	int scene_id;

	int update_cnt = 0;

	SOCKET sock; 
	char nickname[20];
	int count_name = 0;

	void ChangeScene(int);
public:
	Framework();
	~Framework();

	void Render() const;
	void Update();

	int GetSceneId() { return scene_id; }
	void SetNickname(WPARAM wparam);

	PlayScene* GetPlayScene(){ return play_scene; }
};
#endif