#include "Framework.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE 50

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
		new_scene = new StartScene(nickname);
		scene_id = START;
		break;
	case LOBBY:
		new_scene = new LobbyScene(sock, nickname);
		scene_id = LOBBY;
		break;
	case PLAY:
		new_scene = play_scene;
		scene_id = PLAY;
		break;
	case RESULT:
		new_scene = new InterimScene(sock, nickname);
		scene_id = RESULT;
		break;
	case GAMEOVER:
		scene_id = GAMEOVER;
		break;
	case END:
		scene_id = END;
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
	int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	//소켓 생성 
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//if (sock == INVALID_SOCKET) err_quit2("socket()");

	//connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	HANDLE hThread = CreateThread(NULL, 0, RecvThread, (LPVOID)sock, 0, NULL);

	if (hThread == NULL) {
		closesocket(sock);
	}
	else {
		CloseHandle(hThread);

	}

	scene = new StartScene(nickname);
	scene_id = START;
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
	scene->Update(sock, nickname);
	ChangeScene(scene->ChangeScene());
}

void Framework::SetNickname(WPARAM wparam)
{
	count_name = strlen(nickname);
	// 문자 입력 수정 필요
	if (wparam >= 'A' && wparam < 'z' && count_name < 19) {
		nickname[count_name] = (TCHAR)wparam;
		nickname[count_name + 1] = '\0';
	}
}
