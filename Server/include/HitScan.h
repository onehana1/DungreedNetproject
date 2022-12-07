#pragma once
#ifndef _hitscan
#define _hitscan
#include "Character.h"
#include "Missile.h"


class HitScanner
{
public:
	void operator()(Character* attacker, Character* victim);
	void operator()(Missile* attacker, Character* victim, MissileManager* missile_manager );
	int calc_damage(const Character* attacker, const Character* victim);
	int calc_damage(const Missile* attacker, const Character* victim);
};

#endif