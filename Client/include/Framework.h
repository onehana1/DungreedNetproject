#pragma once
#pragma once
#ifndef _framework
#define _framework
#include "Scene.h"

class Framework
{
private:
	Scene* scene;
	PlayScene* play_scene;

	int update_cnt = 0;

	void ChangeScene(int);
public:
	Framework();
	~Framework();

	void Render() const;
	void Update();
};
#endif