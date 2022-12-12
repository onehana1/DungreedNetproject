#include "Scene.h"

Scene::Scene()
{
	try {

		dungeon = new Dungeon;
		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			player[i] = new Player(dungeon);
		}
		monster_manager = new MonsterManager(dungeon );
		missile_manager = new MissileManager;
		for (int i = 0; i < PLAYER_NUM; ++i) {
			weapon[i] = new Weapon(dungeon, player[i]);//const POINT mouse
		}
		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			player[i]->PlaceWithDungeonLeft(dungeon);
		}
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

Scene::Scene(const int dungeon_id)
{
	try {
		dungeon = new Dungeon(dungeon_id);
		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			player[i] = new Player(dungeon);
		}
		monster_manager = new MonsterManager(dungeon );
		missile_manager = new MissileManager;
		for (int i = 0; i < PLAYER_NUM; ++i) {
			weapon[i] = new Weapon(dungeon, player[i]);//const POINT mouse
		}

		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			player[i]->PlaceWithDungeonLeft(dungeon);
		}
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

Scene::~Scene()
{
	delete[] player;
	delete dungeon;
	delete[] weapon;
	delete monster_manager;
	delete missile_manager;
}

HRESULT Scene::Init()
{
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		player[i]->Init(dungeon);
	}
	monster_manager->Init(dungeon );
	missile_manager->Init();
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		weapon[i]->Init(dungeon, player[i], DungeonSize);
	}
	update_cnt = 0;

	return S_OK;
}


void Scene::Update()
{
	// player, monster 업데이트 루틴
	//
	/*printf("update 시작\n");
	for (int i = 0; i < 3; i++)
		InputUpdate(TestPlayer[i]->info);
	printf("update 끝\n");*/
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		player[i]->Update(dungeon, weapon[i], missile_manager);
	}

	monster_manager->Update(dungeon, player, missile_manager);	// 수정 필요
	missile_manager->Update(dungeon);
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		weapon[i]->Update(player[i], {0, 0});	// 플레이어들에게서 받은 마우스 좌표 넣어 줘야 함
	}

	if (Check_Dun_Change[0] && Check_Dun_Change[1] && Check_Dun_Change[2]) {
		Check_Dun_Change[0] = false;
		Check_Dun_Change[1] = false;
		Check_Dun_Change[2] = false;
		printf("던전바꾸기 실행");
		GoNextDungeon();
		
	}
	HitUpdate();
	DungeonChangeProc();
}


void Scene::Send()
{
	//printf("play sending \n tlqkf\n");
	SC_PLAYER_INPUT_INFO_PACKET my_packet;
	my_packet.size = sizeof(SC_PLAYER_INPUT_INFO_PACKET);
	my_packet.type = SC_PLAY;
	for (int i = 0; i < player_list.size(); ++i) {
		my_packet.p_info[i].ID = i;
		
		my_packet.p_info[i].PPos = player[i]->GetPos();
		my_packet.p_info[i].MPos = player[i]->GetInput().mouse.mPos;
		
		my_packet.p_info[i].hp = player[i]->Gethp();
		my_packet.p_info[i].killMonster = player[i]->GetKillMonster();

		my_packet.p_info[i].isMove = player[i]->isMove();
		my_packet.p_info[i].isAttack = player[i]->GetisAttack();
		my_packet.p_info[i].isMisile = player[i]->GetisMisile();
		my_packet.p_info[i].isDash = player[i]->GetisDash();
		
		my_packet.p_info[i].Direciton = player[i]->GetDirection();

		player[i]->ResetAMD();
	}

	for (int i = 0; i < PLAYER_NUM; ++i) {
		if (player_list[i]->GetState() != UNCONNECT) {
			send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
		}
	}

	monster_manager->Send();
	missile_manager->Send();
}

void Scene::DungeonChangeProc()
{
	/*
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
			*/
	if (update_cnt++ % 1000 == 0)
		monster_manager->Appear(5);
}

void Scene::InputUpdate(PLAYER_INPUT_INFO info)
{
	if (player[info.ID]) {
		player[info.ID]->SetInput(info);
	}
}

void Scene::UpdateInfo(int num, Player* player)
{
	//SC_INFO[num].IsMisile = player->GetMisile();
	player->UpdateInfo(&SC_INFO[num]);  // 필요한 정보 쏙쏙 골라담기 
}

void Scene::HitUpdate()
{
	for (int i = 0; i < PLAYER_NUM; ++i) {
		for (auto* monster : monster_manager->monsters)
		{
			if (monster->IsAppeared()) {
				HitScan(player[i], monster);
				HitScan(monster, player[i]);
				for (auto* missile : missile_manager->missiles)
					if (missile->host == player[i])
						HitScan(missile, monster, missile_manager);
					else
						HitScan(missile, player[i], missile_manager);
			}
		}
	}
}

void Scene::GoNextDungeon()
{
	try {
		ChangeDungeon(dungeon->next_dungeon_id);
		Init();
		for (int i = 0; i < PLAYER_NUM; ++i) {
			player[i]->PlaceWithDungeonLeft(dungeon);
		}
		printf("다음 던전\n");
	}
	catch (const TCHAR* error_message) {
		printf("다음 던전 안됨\n");

		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

void Scene::GoPrevDungeon()	// 사용 안함
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

void Scene::ChangeDungeon(const int dungeon_id)
{
	if (!dungeon_id)
		throw L"ChangeDungeon_dungeon_id was 0";
	delete(dungeon);
	dungeon = new Dungeon(dungeon_id);
}

