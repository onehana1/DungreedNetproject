#pragma once
#pragma once
#ifndef _framework
#define _framework
#include "Scene.h"
#include "Common.h"

class Framework
{
private:
	Scene* scene;
	PlayScene* play_scene;

	int update_cnt = 0;

	SOCKET sock; 
	void ChangeScene(int);
public:
	Framework();
	~Framework();

	void Render() const;
	void Update();
};
#endif