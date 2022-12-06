#include "Monster.h"

void Monster::Update(const Dungeon* dungeon, const Player* player,MissileManager* missile_manager)
{
	if (is_appeared) {
		AutoAction(dungeon, player, missile_manager);
		// Die 루틴
		ForceGravity(dungeon);
		ForceGravity(dungeon);

		if (!is_attacking && atk_delay)
			--atk_delay;
		--former_atk_delay;
	}
}

void Monster::AutoAction(const Dungeon* dungeon, const Player* player,  MissileManager* missile_manager)
{
	if (!remain_update_cnt_to_change_policy) {
		//
		is_attacking = false;
		atk_delay = 0;
		former_atk_delay = 0;
		//
		ChooseNewPolicy();
	}
	else {
		FollowPolicy(dungeon, player, missile_manager);
		--remain_update_cnt_to_change_policy;
	}
}

void Monster::FollowPolicy(const Dungeon* dungeon, const Player* player, MissileManager* missile_manager)
{
	MonsterAI monster_ai(this);

	switch (cur_policy) {
	case Policy::STAND:
		monster_ai.Stand();
		break;
	case Policy::MOVE_TO_PLAYER:
		monster_ai.MoveToPlayer(dungeon, player);
		break;
	case Policy::MOVE_FROM_PLAYER:
		monster_ai.MoveFromPlayer(dungeon, player);
		break;
	case Policy::ATTACK:
		monster_ai.Attack(dungeon, player, missile_manager);
		break;
	}
}

void Monster::ChooseNewPolicy()
{
	std::uniform_int_distribution<> uid_chance{ 1, 100 };
	int chance = uid_chance(dre);

	if (chance <= policy_stand.x) {
		cur_policy = Policy::STAND;
		remain_update_cnt_to_change_policy = policy_stand.y;
		animation_name = stand_animation_name;
		is_animation_load_requested = true;
		return;
	}
	chance -= policy_stand.x;
	if (chance <= policy_move_to_player.x) {
		cur_policy = Policy::MOVE_TO_PLAYER;
		remain_update_cnt_to_change_policy = policy_move_to_player.y;
		if (!move_animation_name.empty())
			animation_name = move_animation_name;
		else
			animation_name = stand_animation_name;
		is_animation_load_requested = true;
		return;
	}
	chance -= policy_move_to_player.x;
	if (chance <= policy_move_from_player.x) {
		cur_policy = Policy::MOVE_FROM_PLAYER;
		remain_update_cnt_to_change_policy = policy_move_from_player.y;
		if (!move_animation_name.empty())
			animation_name = move_animation_name;
		else
			animation_name = stand_animation_name;
		is_animation_load_requested = true;
		return;
	}
	chance -= policy_move_from_player.x;
	if (chance <= policy_attack.x) {
		//

		if (id == 4000013) {
			std::uniform_int_distribution<> uid_chance{ 1, 100 };
			int chance = uid_chance(dre);
			if (chance < 50) {
				boss_attack2 = false;
			}
			else {
				boss_attack2 = true;
			}
		}
		//
		cur_policy = Policy::ATTACK;
		remain_update_cnt_to_change_policy = policy_attack.y;
		if (!attack_animation_name.empty())
			animation_name = attack_animation_name;
		else
			animation_name = stand_animation_name;
		is_animation_load_requested = true;
		return;
	}
}

void Monster::ForceGravity(const Dungeon* dungeon)
{
	if (!is_floating)
		this->Character::ForceGravity(dungeon);
}

void Monster::Render(HDC scene_dc, const RECT& bit_rect)
{
	if (is_appeared) {
		this->Character::Render(scene_dc, bit_rect);
		this->Character::RenderMonsterHP(scene_dc, bit_rect);
	}
}



MonsterManager::MonsterManager(const Dungeon* dungeon  )
{
	//
	if (dungeon->monster_ids[0] == 4000013) {
		InsertBoss(dungeon, dungeon->monster_ids[0] );
		return;
	}
	//

	for (int i = 0; i < MAX_MONSTER_KIND_IN_DUNGEON; ++i)
		if (dungeon->monster_ids[i])
			Insert(dungeon, dungeon->monster_ids[i], dungeon->monster_nums[i] );
		else
			break;
}

void MonsterManager::Init(const Dungeon* dungeon  )
{
	Clear();

	//
	if (dungeon->monster_ids[0] == 4000013) {
		InsertBoss(dungeon, dungeon->monster_ids[0] );
		return;
	}
	//

	for (int i = 0; i < MAX_MONSTER_KIND_IN_DUNGEON; ++i)
		if (dungeon->monster_ids[i])
			Insert(dungeon, dungeon->monster_ids[i], dungeon->monster_nums[i] );
		else
			break;
}

void MonsterManager::Update(const Dungeon* dungeon, const Player* player  , MissileManager* missile_manager )
{
	for (Monster* monster : monsters) {
		monster->Update(dungeon, player , missile_manager );
		// Die 루틴 : 현재는 죽으면 그냥 출현 취소
		if (monster->is_appeared && monster->IsDied()) {
			monster->is_appeared = false;
			--remain_monster_cnt;
		}
	}
}

void MonsterManager::Render(HDC scene_dc, const RECT& bit_rect) const
{
	for (Monster* monster : monsters)
		monster->Render(scene_dc, bit_rect);
}

MonsterManager::~MonsterManager()
{
	for (auto* monster : monsters)
		delete monster;
}

void MonsterManager::Insert(const Dungeon* dungeon, const int monster_id, int num  )
{
	auto monster_db = BuildDB();
	POINT pos;
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	monster_db->Load(monster_id);

	remain_monster_cnt += num;

	// 이 수치들은 카메라에 대한 천분율
	width *= dungeon->camera_x_half_range / 1000.0f;
	height *= dungeon->camera_y_half_range / 1000.0f;
	x_move_px_double = x_move_px * dungeon->camera_x_half_range / 1000.0f;
	jump_start_power_double = jump_start_power * dungeon->camera_y_half_range / 1000.0f;
	//

	// 시작 위치는 지형 상 핑크색 위치중 랜덤
	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);
	std::uniform_int_distribution<> uid_x{ 0, dungeon->dungeon_width - width };
	std::uniform_int_distribution<> uid_y{ 0, dungeon->dungeon_height / 3 * 2 };

	LoadNeededAnimations();

	while (num--) {
		do {
			pos.x = uid_x(dre);
			pos.y = uid_y(dre);
		} while (!MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 255), pos));

		Monster* monster = new Monster(monster_id, width, height, pos, x_move_px_double, jump_start_power_double,
			hp, atk, def, is_floating, melee_attack, missile_attack,
			policy_stand, policy_move_to_player, policy_move_from_player, policy_attack ); // = new Monster(...)
		monster->SetID(monsters.size());
		monsters.push_back(monster);
	}

	BufferEmpty();
}

void MonsterManager::InsertBoss(const Dungeon* dungeon, const int monster_id  )
{
	auto monster_db = BuildDB();
	POINT pos;
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	monster_db->Load(monster_id);

	remain_monster_cnt += 1;

	// 이 수치들은 카메라에 대한 천분율
	width *= dungeon->camera_x_half_range / 1000.0f;
	height *= dungeon->camera_y_half_range / 1000.0f;
	x_move_px_double = x_move_px * dungeon->camera_x_half_range / 1000.0f;
	jump_start_power_double = jump_start_power * dungeon->camera_y_half_range / 1000.0f;
	//

	pos.x = dungeon->dungeon_width / 2 - width / 2;
	pos.y = dungeon->dungeon_height / 7 * 3 - height / 2;

	LoadNeededAnimations();

	TCHAR start_image_path[FILE_NAME_LEN] = L"animation/";
	lstrcat(start_image_path, stand_animation_name_tstr);
	lstrcat(start_image_path, L"1.png");

	Monster* monster = new Monster(monster_id, width, height, pos, x_move_px_double, jump_start_power_double,
		hp, atk, def, is_floating, melee_attack, missile_attack,
		policy_stand, policy_move_to_player, policy_move_from_player, policy_attack ); // = new Monster(...)
	monsters.push_back(monster);

	BufferEmpty();
}

void MonsterManager::LoadNeededAnimations()
{
	auto animation_db = std::shared_ptr<DB::DataBase>(new DB::DataBase(L"AnimationData.txt"));

	if (animation_ids[0]) {
		animation_db->RegisterField("animation_name", &stand_animation_name_tstr);
		animation_db->Load(animation_ids[0]);
		lstrcpy(stand_animation_name_tstr, stand_animation_name_tstr);
	}
	animation_db->Init();
	if (animation_ids[1]) {
		animation_db->RegisterField("animation_name", &attack_animation_name_tstr);
		animation_db->Load(animation_ids[1]);
		lstrcpy(attack_animation_name_tstr, attack_animation_name_tstr);
	}
	animation_db->Init();
	if (animation_ids[2]) {
		animation_db->RegisterField("animation_name", &move_animation_name_tstr);
		animation_db->Load(animation_ids[2]);
		lstrcpy(move_animation_name_tstr, move_animation_name_tstr);
	}
}

void MonsterManager::BufferEmpty()
{
	stand_animation_name.clear();
	attack_animation_name.clear();
	move_animation_name.clear();
	stand_animation_name_tstr[0] = NULL;
	attack_animation_name_tstr[0] = NULL;
	move_animation_name_tstr[0] = NULL;
}

void MonsterManager::Clear()
{
	for (Monster* monster : monsters)
		delete monster;
	monsters.clear();
}

std::shared_ptr<DB::DataBase> MonsterManager::BuildDB()
{
	auto db = std::shared_ptr<DB::DataBase>(new DB::DataBase(L"MonsterData.txt"));

	db->RegisterField("monster_name", &monster_name);
	db->RegisterField("width", &width);
	db->RegisterField("height", &height);
	db->RegisterField("x_move_px", &x_move_px);
	db->RegisterField("jump_start_power", &jump_start_power);
	db->RegisterField("hp", &hp);
	db->RegisterField("atk", &atk);
	db->RegisterField("def", &def);
	db->RegisterField("is_floating", &is_floating);
	db->RegisterField("melee_attack", &melee_attack);
	db->RegisterField("missile_attack", &missile_attack);
	db->RegisterField("monster_name", &monster_name);
	db->RegisterField("policy_stand", &policy_stand);
	db->RegisterField("policy_move_to_player", &policy_move_to_player);
	db->RegisterField("policy_move_from_player", &policy_move_from_player);
	db->RegisterField("policy_attack", &policy_attack);

	for (int i = 0; i < MONSTER_MAX_ANIMATION_NUM; ++i) {
		std::string field_str = "animation_id";
		std::stringstream idx;
		idx << i;
		field_str += idx.str();
		db->RegisterField(field_str, &animation_ids[i]);
	}
		
	return db;
}

bool MonsterManager::MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt)	// 지형 표시 이미지를 사용해 충돌 확인, 오류 있음
{
	if (pt.x < client.left || pt.y > client.right)
		return false;
	if (pt.y < client.top || pt.y > client.bottom)
		return false;

	if (GetPixel(terrain_dc, pt.x, pt.y) == val)
		return true;
	else
		return false;
}

void MonsterManager::Appear(int num)
{
	// 일정 시간 이후 문제가 생긴다면 어쩌면 여기 문제
	SC_MAKE_MONSTER_PACKET my_packet;
	my_packet.size = sizeof(SC_MAKE_MONSTER_PACKET);
	my_packet.type = SC_MAKE_MONSTER;

	std::shuffle(monsters.begin(), monsters.end(), dre);
	for (Monster* monster : monsters) {
		if (!monster->is_appeared && !monster->IsDied()) {
			monster->is_appeared = true;
			my_packet.monster[num - 1].Direction = monster->GetDirection();
			my_packet.monster[num - 1].ID = monster->GetID();
			//printf("%d Monster ID = %d\n",num - 1, monster->GetID());
			my_packet.monster[num - 1].Pos = monster->GetPos();
			if (--num == 0)
				break;
		}
	}

	for (int i = num; i > 0; --i) {
		my_packet.monster[i - 1].ID = -1;
	}

	for (int i = 0; i < player_list.size(); ++i)
	{
		if (player_list[i]) {
			send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
		}
	}
}