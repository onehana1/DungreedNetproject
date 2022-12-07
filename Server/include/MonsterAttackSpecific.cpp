#include "MonsterAI.h"
#include "math.h"

void MonsterAI::Attack(const Dungeon* dungeon, const Player* player, MissileManager* missile_manager)
{
	int dx;
	int dy;
	RECT atk_rect;
	RECT player_rect = { player->pos.x, player->pos.y, player->pos.x + player->width, player->pos.y + player->height };
	RECT tmp;

	switch (monster->id) {
	case 4000001:
		dx = monster->pos.x - player->pos.x;
		dy = monster->pos.y - player->pos.y;
		if (sqrt(dx * dx + dy * dy) > 500) {
			if (monster->animation_name == monster->attack_animation_name) {
				if (!monster->move_animation_name.empty()) {
					monster->animation_name = monster->move_animation_name;
					monster->is_move = true;
				}
				else {
					monster->animation_name = monster->stand_animation_name;
					monster->is_stand = true;
				}
				monster->is_animation_load_requested = true;
			}
			MoveToPlayer(dungeon, player);
		}
		else if (monster->atk_delay == 0) {
			monster->animation_name = monster->attack_animation_name;
			monster->attack_animation = true;
			monster->is_animation_load_requested = true;


			monster->is_attack = true;
			monster->StartAttack(15, 45, RECT{});
			monster->is_attacking = false;
		}
		else if (monster->former_atk_delay == 0) {
			monster->is_former_attack = true;
			for (double i = -1; i < 1; i += 1.0 / 4.0) {
				missile_manager->Insert(new Missile(monster, monster->pos, monster->width, monster->height / 6 * 5,
					i * pi, 2, 250, TRUE, 1, 25)); //L"animation/BansheeBullet1.png", "BansheeBullet","sound\\Water1.ogg", 0.4
			}
		}
		break;
	case 4000002:
		if (!monster->is_attacking) {
			if (monster->looking_direction)
				atk_rect = { monster->pos.x + monster->width / 3, monster->pos.y, monster->pos.x + monster->width, monster->pos.y + monster->height };
			else
				atk_rect = { monster->pos.x, monster->pos.y, monster->pos.x + monster->width / 3 * 2, monster->pos.y + monster->height };

			if (IntersectRect(&tmp, &atk_rect, &player_rect)) {
				monster->animation_name = monster->attack_animation_name;
				monster->attack_animation = true;
				monster->is_animation_load_requested = true;
				monster->is_attack = true;
				monster->StartAttack(40, 60, atk_rect);
			}
			else {
				if (monster->animation_name == monster->attack_animation_name) {
					if (!monster->move_animation_name.empty()) {
						monster->animation_name = monster->move_animation_name;
						monster->is_move = true;
					}
					else {
						monster->animation_name = monster->stand_animation_name;
						monster->is_stand = true;
					}
					monster->is_animation_load_requested = true;
				}
				MoveToPlayer(dungeon, player);
			}
		}
		else if (std::chrono::duration<double>(std::chrono::system_clock::now() - monster->attack_start).count() > 0.5) {
			monster->FinishAttack();
		}
		break;
	case 4000013:
		if (monster->atk_delay == 0) {
			monster->animation_name = monster->attack_animation_name;
			monster->is_animation_load_requested = true;
			monster->attack_animation = true;

			if (!monster->boss_attack2) {
				monster->is_attack = true;
				monster->boss_attack_id = 0;
				monster->StartAttack(40, 60, RECT{});
			}
			else {
				monster->is_attack = true;
				monster->boss_attack_id = 1;
				monster->StartAttack(10, 11, RECT{});
				monster->attack_animation = false;
				monster->is_animation_load_requested = false;
			}
			monster->is_attacking = false;
		}
		else if (monster->former_atk_delay == 0) {

			if (!monster->boss_attack2) {
				monster->is_former_attack = true;
				monster->boss_attack_id = 0;
				for (double i = -1; i < 1; i += 1.0 / 4.0) {
					missile_manager->Insert(new Missile(monster,
						POINT{monster->pos.x + monster->width / 2, monster->pos.y + monster->height / 2}
						, monster->width / 5, monster->height / 5,
						i * pi, 4, 500, TRUE, 1, 15)); //, L"animation/SkellBossBullet1.png", "SkellBossBullet" ,"sound\\Explosion1.ogg", 0.4//파일이름 중복이니까 숫자로 관리할 수 있도록 구현전  참고용으로 남겨놈 
				}
			}
			else {
				monster->is_former_attack = true;
				monster->boss_attack_id = 1;
				if (player->pos.x + player->width / 2 > monster->pos.x + monster->width / 2) {
					missile_manager->Insert(new Missile(monster,
						POINT{ monster->pos.x + monster->width / 2, monster->pos.y + monster->height / 2 }
						, monster->width / 5, monster->height / 5,
						Degree(POINT{ player->pos.x + player->width / 2, player->pos.y + player->height / 2 },
							POINT{ monster->pos.x + monster->width / 2, monster->pos.y + monster->height / 2 })
						, 4, 500, TRUE, 1, 15)); //, L"animation/SkellBossBullet1.png","SkellBossBullet" , "sound\\Explosion1.ogg", 0.4
		

				}
				else {
					monster->is_former_attack = true;
					monster->boss_attack_id = 2;
					missile_manager->Insert(new Missile(monster,
						POINT{ monster->pos.x + monster->width / 2, monster->pos.y + monster->height / 2 }
						, monster->width / 5, monster->height / 5,
						Degree(POINT{ player->pos.x + player->width / 2, player->pos.y + player->height / 2 },
							POINT{ monster->pos.x + monster->width / 2, monster->pos.y + monster->height / 2 })
						, 4, 500, FALSE, 1, 15));//, L"animation/SkellBossBullet1.png","SkellBossBullet" , "sound\\Explosion1.ogg", 0.8
				}
			}

		}
		break;
	}
}