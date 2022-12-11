#include "HitScan.h"

void HitScanner::operator()(Character* attacker, Character* victim)
{
	//attacker->former_atk_delay <= 0
	if (attacker->IsAttacking() && !attacker->HasAlreadyAttacked(victim)) {
		RECT my_rect = { victim->pos.x, victim->pos.y, victim->pos.x + victim->width, victim->pos.y + victim->height };
		RECT tmp;
		if (IntersectRect(&tmp, &my_rect, &attacker->atk_rect)) {
			attacker->AddAttackVictim(victim);
			victim->hp -= calc_damage(attacker, victim);
			if (victim->hp <= 0)attacker->kill_monster++; //공격받고 죽는 곳 
			victim->red_flash_cnt = 8;
		}
	}
}

void HitScanner::operator()(Missile* attacker, Character* victim, MissileManager* missile_manager )
{
	if (!attacker->HasAlreadyAttacked(victim)) {
		RECT my_rect = { victim->pos.x, victim->pos.y, victim->pos.x + victim->width, victim->pos.y + victim->height };
		RECT tmp;
		if (IntersectRect(&tmp, &my_rect, &attacker->atk_rect)) {
			attacker->AddAttackVictim(victim);
			victim->hp -= calc_damage(attacker, victim);
			
			victim->red_flash_cnt = 8;
			if (attacker->pierce == attacker->attack_victims.size());
				//missile_manager->Delete(attacker);	// 액세스 위반 수정 필요
		}
	}
}

int HitScanner::calc_damage(const Character* attacker, const Character* victim)
{
	return attacker->atk * victim->def / (victim->def + 100);
}

int HitScanner::calc_damage(const Missile* attacker, const Character* victim)
{
	return attacker->atk * victim->def / (victim->def + 100);
}