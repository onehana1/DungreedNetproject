#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::Render() const
{
}

void Scene::Update(SOCKET socket, char* name)
{
}

int Scene::ChangeScene()
{
	return 0;
}

//------------------------------------------------------------------------------------------------------------------

PlayScene::PlayScene()
{
	try {
		animation_manager = new AnimationManager;
		LoadPlayerAniamtion();

		effect_manager = new EffectManager;
		LoadBattleEffect();

		sound_manager = new SoundManager;
		sound_manager->Init();
		//sound_manager->PlayBgm("sound\\bgm1.mp3");
		LoadBattleSound();

		dungeon = new Dungeon;
		player = new Player(dungeon, animation_manager);
		monster_manager = new MonsterManager(dungeon, animation_manager);
		missile_manager = new MissileManager;
		camera = new Camera(dungeon, player);
		crosshair = new Crosshair(camera);
		weapon = new Weapon(camera, player, crosshair, animation_manager);

		player->PlaceWithDungeonLeft(dungeon);
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

PlayScene::PlayScene(const int dungeon_id)
{
	try {
		
		SOCKET Psock;

		animation_manager = new AnimationManager;
		LoadPlayerAniamtion();

		effect_manager = new EffectManager;
		LoadBattleEffect();
		sound_manager = new SoundManager;

		sound_manager->Init();
		//sound_manager->PlayBgm("sound\\bgm1.mp3");
		LoadBattleSound();

		dungeon = new Dungeon(dungeon_id);
		player = new Player(dungeon, animation_manager);
		monster_manager = new MonsterManager(dungeon, animation_manager);
		missile_manager = new MissileManager;
		camera = new Camera(dungeon, player);
		crosshair = new Crosshair(camera);
		weapon = new Weapon(camera, player, crosshair, animation_manager);

		player->PlaceWithDungeonLeft(dungeon);
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

PlayScene::~PlayScene()
{
	delete player;
	delete dungeon;
	delete camera;
	delete crosshair;
	delete weapon;
	delete animation_manager;
	delete monster_manager;
	delete effect_manager;
	delete sound_manager;
	delete missile_manager;
}

HRESULT PlayScene::Init()
{
	player->Init(dungeon, animation_manager);
	monster_manager->Init(dungeon, animation_manager);
	missile_manager->Init();
	camera->Init(dungeon, player);
	crosshair->Init(camera);
	weapon->Init(camera, player, crosshair, animation_manager);

	update_cnt = 0;

	return S_OK;
}

void PlayScene::Render() const
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->Render(dc_set.buf_dc, dc_set.bit_rect);
	player->Render(dc_set.buf_dc, dc_set.bit_rect);
	player->RenderPlayerHP(dc_set.buf_dc, dc_set.bit_rect, camera->Rect());
	monster_manager->Render(dc_set.buf_dc, dc_set.bit_rect);
	missile_manager->Render(dc_set.buf_dc, dc_set.bit_rect);
	crosshair->Render(dc_set.buf_dc, dc_set.bit_rect);
	weapon->Render(dc_set.buf_dc, dc_set.bit_rect);
	effect_manager->Render(dc_set.buf_dc, dc_set.bit_rect);



	DrawBuffer(dc_set.buf_dc, camera->Rect());
}

void PlayScene::Update(SOCKET socket, char* name)
{


	server_sock = socket;
	CS_PLAYER_INPUT_INFO_PACKET my_packet{};
	my_packet.size = sizeof(CS_PLAYER_INPUT_INFO_PACKET);
	my_packet.type = CS_PLAY;


	my_packet.key.a = GetAsyncKeyState('A');
	my_packet.key.s = GetAsyncKeyState('S');
	my_packet.key.d = GetAsyncKeyState('D');
	my_packet.key.space = GetAsyncKeyState(VK_SPACE);

	my_packet.mouse.right = GetAsyncKeyState(VK_RBUTTON);
	my_packet.mouse.left = GetAsyncKeyState(VK_LBUTTON);
	my_packet.mouse.wheel = GetAsyncKeyState(VK_MBUTTON);
	my_packet.mouse.mPos.x = crosshair->pos.x;
	my_packet.mouse.mPos.y = crosshair->pos.y;

	send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

	// player, monster 업데이트 루틴
	player->Update(dungeon, weapon, crosshair, missile_manager, animation_manager, sound_manager, effect_manager);
	monster_manager->Update(dungeon, player, animation_manager, missile_manager, sound_manager);
	missile_manager->Update(dungeon, animation_manager);
	camera->Update(dungeon, player);
	crosshair->Update(camera);
	weapon->Update(player, crosshair, animation_manager);
	effect_manager->Update(animation_manager);
	HitUpdate();
	DungeonChangeProc();
}


int PlayScene::ChangeScene()
{
	if (player_list[0]->GetState()==RESULTING) {
		return 4;
	}
	return 0;


}

void PlayScene::DungeonChangeProc()
{
	if (player->IsOut_Right(dungeon))
		if (monster_manager->AreMonsterAllDied())
			GoNextDungeon();
		else
			player->NoOut(dungeon);
	else if (player->IsOut_Left(dungeon))
		if (monster_manager->AreMonsterAllDied())
			GoPrevDungeon();
		else
			player->NoOut(dungeon);

	//if (update_cnt++ % 1000 == 0)
	//	monster_manager->Appear(5);
}

void PlayScene::HitUpdate()
{
	for (auto* monster : monster_manager->monsters)
		if (monster->IsAppeared()) {
			HitScan(player, monster, sound_manager);
			HitScan(monster, player, sound_manager);
			for (auto* missile : missile_manager->missiles)
				if (missile->host == player)
					HitScan(missile, monster, missile_manager, sound_manager);
				else
					HitScan(missile, player, missile_manager, sound_manager);
		}
}

void PlayScene::GoDungeonResult()
{
}

void PlayScene::GoNextDungeon()
{
	try {
		ChangeDungeon(dungeon->next_dungeon_id);
		Init();
		player->PlaceWithDungeonLeft(dungeon);
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

void PlayScene::GoPrevDungeon()
{
	try {
		ChangeDungeon(dungeon->prev_dungeon_id);
		Init();
		player->PlaceWithDungeonRight(dungeon);
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

void PlayScene::ChangeDungeon(const int dungeon_id)
{
	if (!dungeon_id)
		throw L"ChangeDungeon_dungeon_id was 0";
	delete(dungeon);
	dungeon = new Dungeon(dungeon_id);
}

void PlayScene::LoadPlayerAniamtion()
{
	animation_manager->Insert("player_stand");
	animation_manager->Insert("player_move");
	animation_manager->Insert("RustyGreatSwordAttack");
	animation_manager->Insert("SwordMissile");

	//
	// 던전 구분 않고 미사일 애니메이션들 여기서 로드
	// 나중에 구분하도록 바꾸기
	animation_manager->Insert("BansheeBullet");
	animation_manager->Insert("SkellBossBullet");
}

void PlayScene::LoadBattleSound()
{
	/*sound_manager->InsertEffectSound("sound\\walk.mp3");
	sound_manager->InsertEffectSound("sound\\dash.mp3");
	sound_manager->InsertEffectSound("sound\\jump.mp3");
	sound_manager->InsertEffectSound("sound\\swing1.mp3");
	sound_manager->InsertEffectSound("sound\\Slash2.ogg");
	sound_manager->InsertEffectSound("sound\\Slash8.ogg");
	sound_manager->InsertEffectSound("sound\\Thunder7.ogg");
	sound_manager->InsertEffectSound("sound\\Water1.ogg");
	sound_manager->InsertEffectSound("sound\\Explosion1.ogg");*/
}

void PlayScene::LoadBattleEffect()
{
	animation_manager->Insert("Dust");
}

//------------------------------------------------------------------------------------------------------------------

LobbyScene::LobbyScene()
{
	try {
		image = new Image(L"Background\\LobbyScene.png");
		start_button = new Image(L"Background\\start_button.png");
		crosshair = new Crosshair(image->GetWidth(), image->GetHeight());
		animation_manager = new AnimationManager;
		animation_manager->Insert("player_stand");
		animation_manager->Insert("player_move");
		animation_manager->Insert("Dust");
		effect_manager = new EffectManager;
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

LobbyScene::LobbyScene(SOCKET socket, char* name)
{
	try {
		server_sock = socket;
		CS_LOGIN_INFO_PACKET my_packet{};
		my_packet.size = sizeof(CS_LOGIN_INFO_PACKET);
		my_packet.type = CS_LOGIN;

		strcpy(my_packet.name, "닉네임");
		send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

		image = new Image(L"Background\\LobbyScene.png");
		start_button = new Image(L"Background\\start_button.png");	// 추후 이미지 변경 필요
		crosshair = new Crosshair(image->GetWidth(), image->GetHeight());
		animation_manager = new AnimationManager;
		animation_manager->Insert("player_stand");
		animation_manager->Insert("player_move");
		animation_manager->Insert("Dust");
		effect_manager = new EffectManager;

		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			player[i] = new Player(image->GetWidth(), image->GetHeight(), {45 + 147 * i, 100}, animation_manager);
		}
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

LobbyScene::~LobbyScene()
{
	delete crosshair;
	delete image;
	delete start_button;
	delete animation_manager;
	delete effect_manager;
	delete[] player;
}

void LobbyScene::Render() const
{
	int width = image->GetWidth();
	int height = image->GetHeight();
	InstantDCSet dc_set(RECT{ 0, 0, width, height});

	image->Draw(dc_set.buf_dc, 0, 0, dc_set.bit_rect.right, dc_set.bit_rect.bottom, 0, 0, width, height);
	start_button->Draw(dc_set.buf_dc, dc_set.bit_rect.right - 100, dc_set.bit_rect.bottom - 70, 80, 50, 0, 0, start_button->GetWidth(), start_button->GetHeight());
	crosshair->Render(dc_set.buf_dc, dc_set.bit_rect);
	effect_manager->Render(dc_set.buf_dc, dc_set.bit_rect);

	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(h_wnd, &pos);
	TCHAR lpOut[100];
	TCHAR name[50];
	TCHAR ip[50];
	HFONT hFont, OldFont;	// 폰트 지정

	SetBkMode(dc_set.buf_dc, TRANSPARENT);	// 글자 입력시 배경 투명
	SetTextColor(dc_set.buf_dc, RGB(255, 255, 255));	// 글자 하얀색
	hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));		// 폰트 등 추후 수정
	OldFont = (HFONT)SelectObject(dc_set.buf_dc, hFont);


	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		if (player_list[i] && (player_list[i]->GetState() == IN_LOBBY || player_list[i]->GetState() == READY)) {
			ZeroMemory(lpOut, 100);
			ZeroMemory(name, 50);
			ZeroMemory(ip, 50);
			player[i]->Render(dc_set.buf_dc, dc_set.bit_rect);

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetName(), strlen(player_list[i]->GetName()), name, 50);
			wsprintf(lpOut, TEXT("닉네임 : %s"), name);
			TextOut(dc_set.buf_dc, 45 + 147 * i, 280, lpOut, lstrlen(lpOut));
			
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetIp(), strlen(player_list[i]->GetIp()), ip, 50);
			wsprintf(lpOut, TEXT("IP : %s"), ip);
			TextOut(dc_set.buf_dc, 45 + 147 * i, 300, lpOut, lstrlen(lpOut));

			if (player_list[i]->GetState() == IN_LOBBY)
			{
				TextOut(dc_set.buf_dc, 45 + 147 * i, 320, TEXT("준비중..."), lstrlen(TEXT("준비중...")));
			}
			else 
			{
				TextOut(dc_set.buf_dc, 45 + 147 * i, 320, TEXT("준비 완료!"), lstrlen(TEXT("준비 완료!")));
			}
		}
	}

	SelectObject(dc_set.buf_dc, OldFont);
	DeleteObject(hFont);

	DrawBuffer(dc_set.buf_dc, RECT{ 0, 0, width, height });
}

void LobbyScene::Update(SOCKET socket, char* name)
{
	crosshair->Update(image->GetWidth(), image->GetHeight());
	effect_manager->Update(animation_manager);
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		if (player_list[i] && (player_list[i]->GetState() == IN_LOBBY || player_list[i]->GetState() == READY)) {
			if (player_list[i]->GetState() == READY) {
				player[i]->ChangeStateToMoving();
				player[i]->MatchStateAndAnimation(animation_manager, effect_manager);
			}
			player[i]->UpdateAnimation(animation_manager);
		}
	}

	if(player_list[g_myid]->GetState() != READY){
		if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) {	// 휠 클릭tl 준비 완료 상태로 변함
			POINT pos;
			GetCursorPos(&pos);
			ScreenToClient(h_wnd, &pos);
			int width = image->GetWidth();
			int height = image->GetHeight();

			//if (pos.x > width - 100 && pos.x < width - 20 && pos.y > height - 70 && pos.y < height - 20)
			//{
				printf("SEND READY\n");
				player_list[g_myid]->SetState(READY);
				CS_READY_PACKET my_packet{};
				my_packet.size = sizeof(CS_READY_PACKET);
				my_packet.type = CS_READY;
				send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			//}
		}
	}
}

int LobbyScene::ChangeScene()
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		POINT pos;
		GetCursorPos(&pos);
		ScreenToClient(h_wnd, &pos);
		if (pos.x > 475 && pos.x < 790 && pos.y > 25 && pos.y < 100)
		{
			return 3;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------------

StartScene::StartScene(char* name)
{
	try {
		nickname = name;
		image = new Image(L"Background\\StartScene.png");
		crosshair = new Crosshair(image->GetWidth(), image->GetHeight());
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

StartScene::~StartScene()
{
	delete image;
	delete crosshair;
}

void StartScene::Render() const
{
	int width = image->GetWidth();
	int height = image->GetHeight();
	InstantDCSet dc_set(RECT{ 0, 0, width, height });

	image->Draw(dc_set.buf_dc, 0, 0, dc_set.bit_rect.right, dc_set.bit_rect.bottom, 0, 0, width, height);
	crosshair->Render(dc_set.buf_dc, dc_set.bit_rect);

	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(h_wnd, &pos);

	TCHAR str[128];
	wsprintf(str, TEXT("[프로그램 기준] X: %04d, Y: %04d"), pos.x, pos.y);

	TextOut(dc_set.buf_dc, 10, 30, str, lstrlen(str));

	// 입력하는 id 출력.
	TCHAR name[100];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, nickname, strlen(nickname),name, 100);
	//wsprintf(name, TEXT("닉네임 : %s"), m);

	RECT rc = { client.right - 240, client.bottom - 313, client.right - 100, client.bottom - 293 };
	DrawText(dc_set.buf_dc, name, wcslen(name), &rc, DT_SINGLELINE);

	DrawBuffer(dc_set.buf_dc, RECT{ 0, 0, width, height });
}

void StartScene::Update(SOCKET socket, char* name)
{
	crosshair->Update(image->GetWidth(), image->GetHeight());
}

int StartScene::ChangeScene()
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		POINT pos;
		GetCursorPos(&pos);
		ScreenToClient(h_wnd, &pos);
		if (pos.x > 450 && pos.x < 820 && pos.y > 460 && pos.y < 580)
		{
			return 2;
		}
	}
	return 0;
}

InterimScene::InterimScene()
{
	try {
		image = new Image(L"Background\\InterimResults.png");
		start_button = new Image(L"Background\\start_button.png");
		crosshair = new Crosshair(image->GetWidth(), image->GetHeight());
		animation_manager = new AnimationManager;
		animation_manager->Insert("player_stand");
		animation_manager->Insert("player_move");
		animation_manager->Insert("Dust");
		effect_manager = new EffectManager;
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}

}

InterimScene::InterimScene(SOCKET socket, char* name)
{
	try {
		server_sock = socket;
		CS_LOGIN_INFO_PACKET my_packet{};
		my_packet.size = sizeof(CS_LOGIN_INFO_PACKET);
		my_packet.type = CS_RESULT;

		strcpy(my_packet.name, "닉네임");
		send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

		image = new Image(L"Background\\InterimResults.png");
		start_button = new Image(L"Background\\start_button.png");	// 추후 이미지 변경 필요
		crosshair = new Crosshair(image->GetWidth(), image->GetHeight());
		animation_manager = new AnimationManager;
		animation_manager->Insert("player_stand");
		animation_manager->Insert("player_move");
		animation_manager->Insert("Dust");
		effect_manager = new EffectManager;

		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			player[i] = new Player(image->GetWidth(), image->GetHeight(), { 45 + 147 * i, 100 }, animation_manager);
		}
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}

}

InterimScene::~InterimScene()
{
	delete crosshair;
	delete image;
	delete start_button;
	delete animation_manager;
	delete effect_manager;
	delete[] player;
}

void InterimScene::Render() const
{
	int width = image->GetWidth();
	int height = image->GetHeight();
	InstantDCSet dc_set(RECT{ 0, 0, width, height });

	image->Draw(dc_set.buf_dc, 0, 0, dc_set.bit_rect.right, dc_set.bit_rect.bottom, 0, 0, width, height);
	start_button->Draw(dc_set.buf_dc, dc_set.bit_rect.right - 100, dc_set.bit_rect.bottom - 70, 80, 50, 0, 0, start_button->GetWidth(), start_button->GetHeight());
	crosshair->Render(dc_set.buf_dc, dc_set.bit_rect);
	effect_manager->Render(dc_set.buf_dc, dc_set.bit_rect);

	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(h_wnd, &pos);
	TCHAR lpOut[100];
	TCHAR name[50];
	TCHAR ip[50];
	HFONT hFont, OldFont;	// 폰트 지정

	SetBkMode(dc_set.buf_dc, TRANSPARENT);	// 글자 입력시 배경 투명
	SetTextColor(dc_set.buf_dc, RGB(255, 255, 255));	// 글자 하얀색
	hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));		// 폰트 등 추후 수정
	OldFont = (HFONT)SelectObject(dc_set.buf_dc, hFont);


	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		if (player_list[i] && (player_list[i]->GetState() == RESULTING)) {
			ZeroMemory(lpOut, 100);
			ZeroMemory(name, 50);
			ZeroMemory(ip, 50);
			player[i]->Render(dc_set.buf_dc, dc_set.bit_rect);

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetName(), strlen(player_list[i]->GetName()), name, 50);
			wsprintf(lpOut, TEXT("닉네임 : %s"), name);
			TextOut(dc_set.buf_dc, 45 + 147 * i, 280, lpOut, lstrlen(lpOut));

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetIp(), strlen(player_list[i]->GetIp()), ip, 50);
			wsprintf(lpOut, TEXT("IP : %s"), ip);
			TextOut(dc_set.buf_dc, 45 + 147 * i, 300, lpOut, lstrlen(lpOut));


		}
	}

	SelectObject(dc_set.buf_dc, OldFont);
	DeleteObject(hFont);

	DrawBuffer(dc_set.buf_dc, RECT{ 0, 0, width, height });

}

void InterimScene::Update(SOCKET socket, char* name)
{
	crosshair->Update(image->GetWidth(), image->GetHeight());
	effect_manager->Update(animation_manager);

	server_sock = socket;
	CS_PLAYER_INPUT_INFO_PACKET my_packet{};
	my_packet.size = sizeof(CS_PLAYER_INPUT_INFO_PACKET);
	my_packet.type = CS_RESULT;


	my_packet.key.a = GetAsyncKeyState('A');
	my_packet.key.s = GetAsyncKeyState('S');
	my_packet.key.d = GetAsyncKeyState('D');
	my_packet.key.space = GetAsyncKeyState(VK_SPACE);

	my_packet.mouse.right = GetAsyncKeyState(VK_RBUTTON);
	my_packet.mouse.left = GetAsyncKeyState(VK_LBUTTON);
	my_packet.mouse.wheel = GetAsyncKeyState(VK_MBUTTON);
	my_packet.mouse.mPos.x = crosshair->pos.x;
	my_packet.mouse.mPos.y = crosshair->pos.y;

	send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

	
}

int InterimScene::ChangeScene()
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		POINT pos;
		GetCursorPos(&pos);
		ScreenToClient(h_wnd, &pos);
		if (pos.x > 475 && pos.x < 790 && pos.y > 25 && pos.y < 100)
		{
			return 3;
		}
	}
	return 0;
}
