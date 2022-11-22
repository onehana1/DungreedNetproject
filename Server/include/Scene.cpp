#include "Scene.h"

Scene::Scene()
{
	try {

		dungeon = new Dungeon;
		player = new Player(dungeon);
		monster_manager = new MonsterManager(dungeon );
		missile_manager = new MissileManager;
		//weapon = new Weapon(camera, player, mouse);//const POINT mouse

		player->PlaceWithDungeonLeft(dungeon);
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

Scene::Scene(const int dungeon_id)
{
	try {
		dungeon = new Dungeon(dungeon_id);
		player = new Player(dungeon);
		monster_manager = new MonsterManager(dungeon );
		missile_manager = new MissileManager;
		weapon = new Weapon( player, DungeonSize);

		player->PlaceWithDungeonLeft(dungeon);
	}
	catch (const TCHAR* error_message) {
		MessageBox(h_wnd, error_message, L"Error", MB_OK);
	}
}

Scene::~Scene()
{
	delete player;
	delete dungeon;
	delete weapon;
	delete monster_manager;
	delete missile_manager;
}

HRESULT Scene::Init()
{
	player->Init(dungeon );
	monster_manager->Init(dungeon );
	missile_manager->Init();
	weapon->Init( player, DungeonSize );

	update_cnt = 0;

	return S_OK;
}


void Scene::Update()
{
	// player, monster 업데이트 루틴
	player->Update(dungeon, weapon, missile_manager);
	monster_manager->Update(dungeon, player , missile_manager  );
	missile_manager->Update(dungeon );
	//weapon->Update(player, DungeonSize );
	HitUpdate();
	DungeonChangeProc();
}

void Scene::DungeonChangeProc()
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

	if (update_cnt++ % 1000 == 0)
		monster_manager->Appear(5);
}

void Scene::InputUpdate(CS_PLAYER_INPUT_INFO_PACKET INFO)
{
	//1명의 정보를 받아 스레드로 갱신한다고 가정. 
	player->SC_Update(dungeon,INFO.mouse, INFO.key, &Player_Info[INFO.ID].PPos); //플레이어 좌표를 갱신함

	//몬스터 정보를 갱신함 
}

void Scene::HitUpdate()
{
	for (auto* monster : monster_manager->monsters)
		if (monster->IsAppeared()) {
			HitScan(player, monster);
			HitScan(monster, player  );
			for (auto* missile : missile_manager->missiles)
				if (missile->host == player)
					HitScan(missile, monster, missile_manager  );
				else
					HitScan(missile, player, missile_manager  );
		}
}

void Scene::GoNextDungeon()
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

void Scene::GoPrevDungeon()
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

void Scene::ChangeDungeon(const int dungeon_id)
{
	if (!dungeon_id)
		throw L"ChangeDungeon_dungeon_id was 0";
	delete(dungeon);
	dungeon = new Dungeon(dungeon_id);
}

