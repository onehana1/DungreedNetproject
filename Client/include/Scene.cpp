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

int compare(const void* one, const void* two) {
	if (*(int*)one > *(int*)two)
		return 1;
	else if (*(int*)one < *(int*)two)
		return -1;
	else return 0;
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
		for (int i = 0; i < PLAYER_NUM; ++i) {
			player[i] = new Player(dungeon, animation_manager);
		}
		monster_manager = new MonsterManager(dungeon, animation_manager);
		missile_manager = new MissileManager;
		camera = new Camera(dungeon, player[g_myid]);
		crosshair = new Crosshair(camera);
		for (int i = 0; i < PLAYER_NUM; ++i) {
			weapon[i] = new Weapon(camera, player[i], player[i]->GetInfo().MPos, animation_manager);
		}

		Init();
		//player->PlaceWithDungeonLeft(dungeon);
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
		for (int i = 0; i < PLAYER_NUM; ++i) {
			player[i] = new Player(dungeon, animation_manager);
		}
		monster_manager = new MonsterManager(dungeon, animation_manager);
		missile_manager = new MissileManager;
		camera = new Camera(dungeon, player[0]);
		crosshair = new Crosshair(camera);
		for (int i = 0; i < PLAYER_NUM; ++i) {
			weapon[i] = new Weapon(camera, player[i], player[i]->GetInfo().MPos, animation_manager);
		}

		Init();
		//player->PlaceWithDungeonLeft(dungeon);
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
	for (int i = 0; i < PLAYER_NUM; ++i) {
		player[i]->Init(dungeon, animation_manager);
	}
	monster_manager->Init(dungeon, animation_manager);
	missile_manager->Init(animation_manager, player[0]->GetWidth(), player[0]->GetHeight() / 2);
	camera->Init(dungeon, player[g_myid]);
	
	crosshair->Init(camera);
	for (int i = 0; i < PLAYER_NUM; ++i) {
		weapon[i]->Init(camera, player[i], player[i]->GetInfo().MPos, animation_manager);
	}
	update_cnt = 0;

	return S_OK;
}

void PlayScene::Render() const
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	//==================================
	TCHAR lpOut[100];
	TCHAR name[50];
	int kill;
	int killarr[3][2];
	TCHAR death[50];
	HFONT hFont, OldFont;	// 폰트 지정

	SetBkMode(dc_set.buf_dc, TRANSPARENT);	// 글자 입력시 배경 투명
	SetTextColor(dc_set.buf_dc, RGB(255, 255, 255));	// 글자 하얀색
	hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));		// 폰트 등 추후 수정
	OldFont = (HFONT)SelectObject(dc_set.buf_dc, hFont);






	dungeon->Render(dc_set.buf_dc, dc_set.bit_rect);
	for (int i = 0; i < PLAYER_NUM; ++i) {
		//player[i]->RenderPlayerTOP(dc_set.buf_dc, dc_set.bit_rect, camera->Rect());

		player[i]->Render(dc_set.buf_dc, dc_set.bit_rect);		
		player[i]->RenderPlayerHP(dc_set.buf_dc, dc_set.bit_rect, camera->Rect());
	}
	monster_manager->Render(dc_set.buf_dc, dc_set.bit_rect);
	missile_manager->Render(dc_set.buf_dc, dc_set.bit_rect);
	crosshair->Render(dc_set.buf_dc, dc_set.bit_rect);
	for (int i = 0; i < PLAYER_NUM; ++i) {
		weapon[i]->Render(dc_set.buf_dc, dc_set.bit_rect);
	}
	effect_manager->Render(dc_set.buf_dc, dc_set.bit_rect);

	//==================================
	int temp =0;
	int temp2 = 0;

	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		killarr[i][0] = player[i]->GetKillMonster();
		killarr[i][1] = i;

	}
	{
		for (int i = 0; i < 3 - 1; i++)
		{
			for (int j = 0; j < 3 - 1 - i; j++)
			{
				if (killarr[j][0] < killarr[j + 1][0])
				{
					temp = killarr[j][0];
					temp2 = killarr[j][1];

					killarr[j][0] = killarr[j + 1][0];
					killarr[j][1] = killarr[j + 1][1];

					killarr[j + 1][0] = temp;
					killarr[j + 1][1] = temp2;

				}
			}
		}
	}
		

		//ZeroMemory(death, 50);
		//MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetName(), strlen(player_list[i]->GetName()), death, 50);//playerinfo 받는걸로 바꾸기
		//wsprintf(lpOut, TEXT("죽은수여기다가 : %s"), death);
		//TextOut(dc_set.buf_dc, 45 + 294 , 50 + 10 * i, lpOut, lstrlen(lpOut));
		
	for (int i = 0; i < PLAYER_NUM; ++i) {
		ZeroMemory(lpOut, 100);
		wsprintf(lpOut, TEXT("%dP - 킬 수 : %d"), killarr[i][1] + 1, killarr[i][0]);
		TextOut(dc_set.buf_dc, 45 + 294, 50 + 10 * i, lpOut, lstrlen(lpOut));
	}
	




	SelectObject(dc_set.buf_dc, OldFont);
	DeleteObject(hFont);
	DrawBuffer(dc_set.buf_dc, camera->Rect());
}

void PlayScene::Update(SOCKET socket, char* name)
{
	
	server_sock = socket;
	CS_PLAYER_INPUT_INFO_PACKET my_packet;

	my_packet.size = sizeof(CS_PLAYER_INPUT_INFO_PACKET);

	my_packet.type = CS_PLAY;

	my_packet.p_info.key.a = GetAsyncKeyState('A');
	my_packet.p_info.key.s = GetAsyncKeyState('S');
	my_packet.p_info.key.d = GetAsyncKeyState('D');
	my_packet.p_info.key.space = GetAsyncKeyState(VK_SPACE);

	my_packet.p_info.mouse.right = GetAsyncKeyState(VK_RBUTTON);
	my_packet.p_info.mouse.left = GetAsyncKeyState(VK_LBUTTON);
	my_packet.p_info.mouse.wheel = GetAsyncKeyState(VK_MBUTTON);	// 미사일 막아 놓음
	my_packet.p_info.mouse.mPos.x = crosshair->pos.x;
	my_packet.p_info.mouse.mPos.y = crosshair->pos.y;

	send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

	// player, monster 업데이트 루틴
	for (int i = 0; i < PLAYER_NUM; ++i) {
		player[i]->Update(dungeon, weapon[i], missile_manager, animation_manager, sound_manager, effect_manager);
	}
	monster_manager->Update(dungeon, player[0], animation_manager, missile_manager, sound_manager);
	missile_manager->Update(dungeon, animation_manager);
	camera->Update(dungeon, player[g_myid]);
	crosshair->Update(camera);
	for (int i = 0; i < PLAYER_NUM; ++i) {
		weapon[i]->Update(player[i], player[i]->GetInfo().MPos, animation_manager);
	}
	effect_manager->Update(animation_manager);
	//HitUpdate();
	DungeonChangeProc();
}


int PlayScene::ChangeScene()
{
	/*for (int i = 0; i < PLAYER_NUM; ++i)
		if (player_list[i]->GetState() == RESULTING) {
			return 4;
		}*/
	return 0;

}

void PlayScene::UpdateInfo(Player* player)
{
	/*
	//SC_INFO[num].IsMisile = player->GetMisile();
	player->UpdateInfo(&my_packet);  // 필요한 정보 쏙쏙 골라담기 

	my_packet.PPos = player->GetPos();
	switch (player->GetState()) {
	case  0:	my_packet.State = playing_State::DOWN; break;
	case  1:	my_packet.State = playing_State::UP; break;
	case  2:my_packet.State = playing_State::STANDING; break;
	case  3:my_packet.State = playing_State::MOVING; break;
	case 4:my_packet.State = playing_State::DOWNJUMP; break;
	default: my_packet.State = playing_State::STANDING; break;

	}

	//player->animation_name = animation_name;
	my_packet.hp = player->GetHp();
	my_packet.IsAttack = player->GetIsAttack();
	//player->IsMove =*/
}

void PlayScene::SetPlayerInfo(PLAYER_INFO p_info[PLAYER_NUM])
{
    for (int i = 0; i < PLAYER_NUM; ++i)
    {
        player[i]->SetPlayerInfo(p_info[i]);
    }
	//player[p_info[0].ID]->SetPlayerInfo(p_info[0]);
}

void PlayScene::DeleteMissile(int id)
{
	missile_manager->Delete(id);
}

void PlayScene::UpdateMissile(MISSILE_INFO missile[MISSILE_NUM])
{
	missile_manager->miss_info.clear();
	for (int i = 0; i < MISSILE_NUM; ++i) {
		if (missile[i].id != -1) {
			missile_manager->miss_info.push_back(missile[i]);
		}
	}
}

void PlayScene::DungeonChangeProc()
{
	/*
	if (player[0]->GetState()==RESULTING)
		GoNextDungeon();
		*/
	for (int i = 0; i < PLAYER_NUM; ++i) {
		if (player[i]->IsOut_Right(dungeon))
			player[i]->NoOut(dungeon);
		else if (player[i]->IsOut_Left(dungeon))
				player[i]->NoOut(dungeon);
	}
			
	//if (update_cnt++ % 1000 == 0)
	//	monster_manager->Appear(5);
}

void PlayScene::HitUpdate()
{
	for (auto* monster : monster_manager->monsters)
		if (monster->IsAppeared()) {
			for (int i = 0; i < PLAYER_NUM; ++i) {
				HitScan(player[i], monster, sound_manager);
				HitScan(monster, player[i], sound_manager);
				for (auto* missile : missile_manager->missiles)
					if (missile->host == player[i])
						HitScan(missile, monster, missile_manager, sound_manager);
					else
						HitScan(missile, player[i], missile_manager, sound_manager);
			}
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
		for (int i = 0; i < PLAYER_NUM; ++i) {
			player[i]->PlaceWithDungeonLeft(dungeon);
		}
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
		//player->PlaceWithDungeonRight(dungeon);
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
			wsprintf(lpOut, TEXT("닉네임 : %dP"), i+1);
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

	if (g_myid == -1) { return; }

	if(player_list[g_myid]->GetState() != READY){
		if (GetAsyncKeyState(VK_MBUTTON) || GetAsyncKeyState(VK_SPACE) & 0x8000) {		// 휠 클릭tl 준비 완료 상태로 변함
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
		//p가 지금 result 창에 있다만 알려준다.
	/*	server_sock = socket;
		P_STATE my_packet{};
		my_packet.size = sizeof(P_STATE);
		my_packet.type = CS_RESULT;
		my_packet.info.state = 1;

		send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);*/


		image = new Image(L"Background\\InterimResults.png");

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
	
	crosshair->Render(dc_set.buf_dc, dc_set.bit_rect);
	effect_manager->Render(dc_set.buf_dc, dc_set.bit_rect);

	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(h_wnd, &pos);
	TCHAR lpOut[100];
	TCHAR name[50];
	int kill;
	TCHAR death[50];

	TCHAR ip[50];
	HFONT hFont, OldFont;	// 폰트 지정

	SetBkMode(dc_set.buf_dc, TRANSPARENT);	// 글자 입력시 배경 투명
	SetTextColor(dc_set.buf_dc, RGB(255, 255, 255));	// 글자 하얀색
	hFont = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));		// 폰트 등 추후 수정
	OldFont = (HFONT)SelectObject(dc_set.buf_dc, hFont);


	TCHAR str[128];
	wsprintf(str, TEXT("[프로그램 기준] X: %04d, Y: %04d"), pos.x, pos.y);



		ZeroMemory(lpOut, 100);
		//ZeroMemory(name, 50);
		//MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[g_myid]->GetName(), strlen(player_list[g_myid]->GetName()), name, 50);
		//wsprintf(lpOut, TEXT("닉네임 : %s"), name);
		//TextOut(dc_set.buf_dc, 45 + 147, 50, lpOut, lstrlen(lpOut));

		//ZeroMemory(kill, 50);
		
		kill = player_list[g_myid]->GetKillMonster();
		wsprintf(lpOut, TEXT("%d"), kill);
		TextOut(dc_set.buf_dc, 45 + 147, 250, lpOut, lstrlen(lpOut));


		ZeroMemory(death, 50);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[g_myid]->GetName(), strlen(player_list[g_myid]->GetName()), death, 50);//playerinfo 받는걸로 바꾸기
		wsprintf(lpOut, TEXT("%dP : %s"), g_myid + 1 , death);
		TextOut(dc_set.buf_dc, 45 + 147, 150, lpOut, lstrlen(lpOut));

		/*if (player_list[i] && (player_list[i]->GetState() == RESULTING)) {
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


		}*/
	

	SelectObject(dc_set.buf_dc, OldFont);
	DeleteObject(hFont);

	DrawBuffer(dc_set.buf_dc, RECT{ 0, 0, width, height });

}

void InterimScene::Update(SOCKET socket, char* name)
{
	crosshair->Update(image->GetWidth(), image->GetHeight());
	effect_manager->Update(animation_manager);

	/*server_sock = socket;
	P_STATE my_packet{};
	my_packet.size = sizeof(P_STATE);
	my_packet.type = CS_RESULT;
	my_packet.info.state = 1;

	send(server_sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);*/

	
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

	//for (int i = 0; i < PLAYER_NUM; ++i)
	//	if (player_list[i]->GetState() == RESULTING) {
	//		return 4;
	//	}

	return 0;
}
