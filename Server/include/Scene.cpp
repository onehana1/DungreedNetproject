#include "Scene.h"

Scene::Scene()
{
	try {

		dungeon = new Dungeon;
		player = new Player(dungeon);
		monster_manager = new MonsterManager(dungeon );
		missile_manager = new MissileManager;
		weapon = new Weapon(dungeon, player);//const POINT mouse

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
		weapon = new Weapon(dungeon, player);

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
	//
	printf("update 시작\n");
	for (int i = 0; i < 3; i++)
		InputUpdate(TestPlayer[i]->info);
	printf("update 끝\n");
	player->Update(dungeon, weapon, missile_manager);

	monster_manager->Update(dungeon, player, missile_manager);
	missile_manager->Update(dungeon );
	weapon->Update(player, {0, 0});	// 플레이어들에게서 받은 마우스 좌표 넣어 줘야 함
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

void Scene::InputUpdate(PLAYER_INPUT_INFO INFO)
{
	//1명의 정보를 받아 스레드로 갱신한다고 가정. 
	// 보낼때는 배열로 4명의 정보를 한번에 보냄 ( 플레이어 클래스로 관리 후 필요한 정보만 INFO에 담굴것 ) 
	/*player->SC_Update(dungeon, INFO.mouse, INFO.key, &Player_Info[INFO.ID].PPos); //플레이어 좌표를 갱신함 */

	//TestPlayer[INFO.ID]->SC_Update2(dungeon, weapon, missile_manager, INFO.key, INFO.mouse); //이 계산된 좌표를 보낸다. 받은 키, 마우스 값을 넣어 계산한다.
	//UpdateInfo(INFO.ID, player[INFO.ID])
	//몬스터 정보를 갱신함 

	 //이 계산된 좌표를 보낸다. 받은 키, 마우스 값을 넣어 계산한다.
	TestPlayer[INFO.ID]->SC_Update2(dungeon, weapon, missile_manager, INFO.key, INFO.mouse); //TestPlayer[ID] ID가 클라정보
	UpdateInfo(INFO.ID, TestPlayer[INFO.ID]); // 보낼 정보들 골라담기 
	//printf("보내보기");
	//몬스터 정보를 갱신함 -> Scene안의 monster리스트를 통해 관리 * 던전 갱신시 MakeMonster정보 전송필요. 
}

void Scene::UpdateInfo(int num, Player* player)
{
	//SC_INFO[num].IsMisile = player->GetMisile();
	player->UpdateInfo(&SC_INFO[num]);  // 필요한 정보 쏙쏙 골라담기 
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

