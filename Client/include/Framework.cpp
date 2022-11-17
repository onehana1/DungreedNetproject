#include "Framework.h"

enum Scene_num {
	NONE, START, LOBBY, PLAY, RESULT, GAMEOVER, END
};

void Framework::ChangeScene(int scene_num)
{
	Scene* new_scene;
	switch (scene_num) {
	case NONE:
		return;
	case START:
		new_scene = new StartScene;
		break;
	case LOBBY:
		new_scene = new LobbyScene;
		break;
	case PLAY:
		new_scene = play_scene;
		break;
	case RESULT:
		break;
	case GAMEOVER:
		break;
	case END:
		break;
	default:
		return;
	}

	if (scene && scene != play_scene) {
		delete scene;
	}
	scene = new_scene;
}

Framework::Framework()
{
	scene = new StartScene;
	play_scene = new PlayScene;	
}

Framework::~Framework()
{
	if (scene) { delete scene; }
	if (play_scene) { delete play_scene; }
}

void Framework::Render() const
{
	scene->Render();
}

void Framework::Update()
{
	scene->Update();
	ChangeScene(scene->ChangeScene());

	
}
