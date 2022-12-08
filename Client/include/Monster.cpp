#include "Monster.h"

void Monster::Update(const Dungeon* dungeon, const Player* player, AnimationManager* animation_manager, MissileManager* missile_manager, SoundManager* sound_manager)
{
	if (is_appeared) {
		//AutoAction(dungeon, player, animation_manager, missile_manager, sound_manager);
		// Die ��ƾ
		//ForceGravity(dungeon);
		//ForceGravity(dungeon);
		UpdateAnimation(animation_manager);

		if (!is_attacking && atk_delay)
			--atk_delay;
		--former_atk_delay;
	}
}

void Monster::AutoAction(const Dungeon* dungeon, const Player* player, AnimationManager* animation_manager, MissileManager* missile_manager, SoundManager* sound_manager)
{
	if (!remain_update_cnt_to_change_policy) {
		animation.Stop();
		//
		is_attacking = false;
		atk_delay = 0;
		former_atk_delay = 0;
		//
		ChooseNewPolicy();
	}
	else {
		FollowPolicy(dungeon, player, animation_manager, missile_manager, sound_manager);
		--remain_update_cnt_to_change_policy;
	}
}

void Monster::FollowPolicy(const Dungeon* dungeon, const Player* player, AnimationManager* animation_manager, MissileManager* missile_manager, SoundManager* sound_manager)
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
		monster_ai.Attack(dungeon, player, animation_manager, missile_manager, sound_manager);
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



MonsterManager::MonsterManager(const Dungeon* dungeon, AnimationManager* animation_manager)
{
	//
	if (dungeon->monster_ids[0] == 4000013) {
		InsertBoss(dungeon, dungeon->monster_ids[0], animation_manager);
		return;
	}
	//

	for (int i = 0; i < MAX_MONSTER_KIND_IN_DUNGEON; ++i)
		if (dungeon->monster_ids[i])
			Insert(dungeon, dungeon->monster_ids[i], dungeon->monster_nums[i], animation_manager);
		else
			break;
}

void MonsterManager::Init(const Dungeon* dungeon, AnimationManager* animation_manager)
{
	Clear();

	//
	if (dungeon->monster_ids[0] == 4000013) {
		InsertBoss(dungeon, dungeon->monster_ids[0], animation_manager);
		return;
	}
	//

	for (int i = 0; i < MAX_MONSTER_KIND_IN_DUNGEON; ++i)
		if (dungeon->monster_ids[i])
			Insert(dungeon, dungeon->monster_ids[i], dungeon->monster_nums[i], animation_manager);
		else
			break;
}

void MonsterManager::Update(const Dungeon* dungeon, const Player* player, AnimationManager* animation_manager, MissileManager* missile_manager, SoundManager* sound_manager)
{
	for (Monster* monster : monsters) {
		monster->Update(dungeon, player, animation_manager, missile_manager, sound_manager);
		// Die ��ƾ : ����� ������ �׳� ���� ���
		//if (monster->is_appeared && monster->IsDied()) {
			//monster->is_appeared = false;
			//--remain_monster_cnt;
		//}
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

void MonsterManager::Insert(const Dungeon* dungeon, const int monster_id, int num, AnimationManager* animation_manager)
{
	auto monster_db = BuildDB();
	POINT pos;
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	monster_db->Load(monster_id);

	remain_monster_cnt += num;

	// �� ��ġ���� ī�޶� ���� õ����
	width *= dungeon->camera_x_half_range / 1000.0f;
	height *= dungeon->camera_y_half_range / 1000.0f;
	x_move_px_double = x_move_px * dungeon->camera_x_half_range / 1000.0f;
	jump_start_power_double = jump_start_power * dungeon->camera_y_half_range / 1000.0f;
	//

	// ���� ��ġ�� ���� �� ��ũ�� ��ġ�� ����
	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);
	std::uniform_int_distribution<> uid_x{ 0, dungeon->dungeon_width - width };
	std::uniform_int_distribution<> uid_y{ 0, dungeon->dungeon_height / 3 * 2 };

	LoadNeededAnimations();
	
	TCHAR start_image_path[FILE_NAME_LEN] = L"animation/";
	lstrcat(start_image_path, stand_animation_name_tstr);
	lstrcat(start_image_path, L"1.png");

	Tstr2Str(stand_animation_name_tstr, stand_animation_name);
	Tstr2Str(attack_animation_name_tstr, attack_animation_name);
	Tstr2Str(move_animation_name_tstr, move_animation_name);

	if (!stand_animation_name.empty())
		animation_manager->Insert(stand_animation_name);
	if (!attack_animation_name.empty())
		animation_manager->Insert(attack_animation_name);
	if (!move_animation_name.empty())
		animation_manager->Insert(move_animation_name);

	while (num--) {
		do {
			pos.x = uid_x(dre);
			pos.y = uid_y(dre);
		} while (!MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 255), pos));

		Monster* monster = new Monster(monster_id, width, height, pos, x_move_px_double, jump_start_power_double,
			hp, atk, def, is_floating, melee_attack, missile_attack,
			stand_animation_name, attack_animation_name, move_animation_name, start_image_path,
			policy_stand, policy_move_to_player, policy_move_from_player, policy_attack, animation_manager); // = new Monster(...)
		monsters.push_back(monster);
	}

	BufferEmpty();
}

void MonsterManager::InsertBoss(const Dungeon* dungeon, const int monster_id, AnimationManager* animation_manager)
{
	auto monster_db = BuildDB();
	POINT pos;
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	monster_db->Load(monster_id);

	remain_monster_cnt += 1;

	// �� ��ġ���� ī�޶� ���� õ����
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

	Tstr2Str(stand_animation_name_tstr, stand_animation_name);
	Tstr2Str(attack_animation_name_tstr, attack_animation_name);
	Tstr2Str(move_animation_name_tstr, move_animation_name);

	if (!stand_animation_name.empty())
		animation_manager->Insert(stand_animation_name);
	if (!attack_animation_name.empty())
		animation_manager->Insert(attack_animation_name);
	if (!move_animation_name.empty())
		animation_manager->Insert(move_animation_name);

	Monster* monster = new Monster(monster_id, width, height, pos, x_move_px_double, jump_start_power_double,
		hp, atk, def, is_floating, melee_attack, missile_attack,
		stand_animation_name, attack_animation_name, move_animation_name, start_image_path,
		policy_stand, policy_move_to_player, policy_move_from_player, policy_attack, animation_manager); // = new Monster(...)
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

bool MonsterManager::MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt)	// ���� ǥ�� �̹����� ����� �浹 Ȯ��, ���� ����
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
	// ���� �ð� ���� ������ ����ٸ� ��¼�� ���� ����
	std::shuffle(monsters.begin(), monsters.end(), dre);
	for (Monster* monster : monsters)
		if (!monster->is_appeared && !monster->IsDied()) {
			monster->is_appeared = true;
			if (--num == 0)
				break;
		}
}

void MonsterManager::Appear(MAKE_MONSTER monster[5])
{
	for (int i = 0; i < 5; ++i)
	{
		if (monster[i].ID != -1) {
			monsters[monster[i].ID]->is_appeared = true;
			monsters[monster[i].ID]->SetDirection(monster[i].Direction);
			monsters[monster[i].ID]->SetPos(monster[i].Pos);
		}
	}
}

void MonsterManager::UpdateMonsterInfo(MONSTER_INFO_MANAGER monster_info[20], Player* player[3], MissileManager* missile_manager, AnimationManager* animation_manager)
{
	bool check_died[20]{};
	for (int i = 0; i < 20; ++i) {
		check_died[i] = true;
	}
	for (int i = 0; i < 20; ++i) {
		int id = monster_info[i].id;
		
		if (id != -1) {
			//monsters[id]->is_appeared = true;
			check_died[id] = false;
			if (monsters[id]->hp > monster_info[i].hp) {
				monsters[id]->red_flash_cnt = 8;
			}
			monsters[id]->hp = monster_info[i].hp;
			monsters[id]->pos = monster_info[i].PPos;
			monsters[id]->looking_direction = monster_info[i].Direction;

			if (monster_info[i].isMove) {
				monsters[id]->animation_name = monsters[id]->move_animation_name;
				monsters[id]->is_animation_load_requested = true;
			}
			else if (monster_info[i].isStand) {
				monsters[id]->animation_name = monsters[id]->stand_animation_name;
				monsters[id]->is_animation_load_requested = true;
			}
			else if (monster_info[i].attack_animation) {
				monsters[id]->animation_name = monsters[id]->attack_animation_name;
				monsters[id]->is_animation_load_requested = true;
			}

			if (monster_info[i].isFormerAttack) {
				int player_num = monster_info[i].player_id;
/*
				switch (monsters[id]->id) {
				case 4000001:
				{
					for (double i = -1; i < 1; i += 1.0 / 4.0) {
						missile_manager->Insert(new Missile(monsters[id], monsters[id]->pos, monsters[id]->width, monsters[id]->height / 6 * 5,
							i * pi, 2, 250, TRUE, 1, 25, L"animation/BansheeBullet1.png", "BansheeBullet", animation_manager,
							"sound\\Water1.ogg", 0.4));
					}
					break;
				}
				case 4000013:
				{
					if (monster_info[i].BossAttackID == 0) {
						missile_manager->Insert(new Missile(monsters[id],
							POINT{ monsters[id]->pos.x + monsters[id]->width / 2, monsters[id]->pos.y + monsters[id]->height / 2 }
							, monsters[id]->width / 5, monsters[id]->height / 5,
							i * pi, 4, 500, TRUE, 1, 15, L"animation/SkellBossBullet1.png", "SkellBossBullet", animation_manager,
							"sound\\Explosion1.ogg", 0.4));
					}
					else if (monster_info[i].BossAttackID == 1) {
						missile_manager->Insert(new Missile(monsters[id],
							POINT{ monsters[id]->pos.x + monsters[id]->width / 2, monsters[id]->pos.y + monsters[id]->height / 2 }
							, monsters[id]->width / 5, monsters[id]->height / 5,
							Degree(POINT{ player[player_num]->GetPos().x + player[player_num]->GetWidth() / 2, player[player_num]->GetPos().y + player[player_num]->GetHeight() / 2 },
								POINT{ monsters[id]->pos.x + monsters[id]->width / 2, monsters[id]->pos.y + monsters[id]->height / 2 })
							, 4, 500, TRUE, 1, 15, L"animation/SkellBossBullet1.png",
							"SkellBossBullet", animation_manager, "sound\\Explosion1.ogg", 0.4));
					}
					else if (monster_info[i].BossAttackID == 2) {
						missile_manager->Insert(new Missile(monsters[id],
							POINT{ monsters[id]->pos.x + monsters[id]->width / 2, monsters[id]->pos.y + monsters[id]->height / 2 }
							, monsters[id]->width / 5, monsters[id]->height / 5,
							Degree(POINT{ player[player_num]->GetPos().x + player[player_num]->GetWidth() / 2, player[player_num]->GetPos().y + player[player_num]->GetHeight() / 2 },
								POINT{ monsters[id]->pos.x + monsters[id]->width / 2, monsters[id]->pos.y + monsters[id]->height / 2 })
							, 4, 500, FALSE, 1, 15, L"animation/SkellBossBullet1.png",
							"SkellBossBullet", animation_manager, "sound\\Explosion1.ogg", 0.8));
					}
					break;
				}
				
				}
				*/
			}
		}
		
	}

	for (int i = 0; i < 20; ++i) {
		if (check_died[i]) {
			monsters[i]->is_appeared = false;
		}
	}
}
