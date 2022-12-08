#include "Missile.h"

void Missile::Update(AnimationManager* animation_manager)
{
	/*
	POINT desti_pos;
	if (looking_direction) {
		desti_pos = { static_cast<LONG>(pos.x + speed * cos(radian)), static_cast<LONG>(pos.y - speed * sin(radian)) };
	}
	else {
		desti_pos = { static_cast<LONG>(pos.x - speed * cos(radian)), static_cast<LONG>(pos.y - speed * sin(radian)) };
	}

	pos = desti_pos;

	atk_rect = { pos.x, pos.y, pos.x + width, pos.y + height };
	*/
	UpdateAnimation(animation_manager);
}

void Missile::Render(HDC scene_dc, const RECT& bit_rect) const
{
	int image_width = image->GetWidth();
	int image_height = image->GetHeight();

	HBITMAP hbm_rotate = RotateImage(scene_dc, image, pi / 2 - radian);
	Image* rotate_image = new Image;
	rotate_image->Attach(hbm_rotate);
	rotate_image->SetTransparentColor(RGB(0, 0, 0));

	if (looking_direction) {
		rotate_image->Draw(scene_dc, pos.x, pos.y, width, height, 0, 0, image_width, image_height);
	}
	else {
		DrawFlip(scene_dc, bit_rect, rotate_image, pos, width, height);
	}
	DeleteObject(hbm_rotate);
	delete rotate_image;
}

bool Missile::IsOut_Left(const Dungeon* dungeon) const
{
	if (pos.x < 0)
		return true;
	else
		return false;
}

bool Missile::IsOut_Right(const Dungeon* dungeon) const
{
	if (pos.x + width > dungeon->dungeon_width)
		return true;
	else
		return false;
}

void Missile::AddAttackVictim(const Character* victim)
{
	attack_victims.push_back(victim);
}

bool Missile::HasAlreadyAttacked(const Character* victim) const
{
	for (const auto& iter : attack_victims)
		if (iter == victim)
			return true;
	return false;
}

bool Missile::IsOutOfRange() const
{
	int dx = pos.x - old_pos.x;
	int dy = pos.y - old_pos.y;
	if (sqrt(dx * dx + dy * dy) > range)
		return true;
	else
		return false;
}

void Missile::UpdateAnimation(AnimationManager* animation_manager)
{
	if (is_animation_load_requested) {
		animation.LoadAnimation(animation_manager, animation_name);
		animation.Play();
		is_animation_load_requested = false;
	}
	else if (animation.IsPlaying()) {
		if (animation.IsEnd())
			animation.Stop();
		else
			animation.Update();
	}

	image = animation.GetImage(animation_manager);
}

void MissileManager::Init(AnimationManager* animation_manager, int w, int h)
{
	for (auto* missile : missiles)
		delete missile;
	missiles.clear();

	missiles.push_back(new Missile(NULL, {0, 0}, 20, 22 / 6 * 5,
		TRUE, L"animation/BansheeBullet1.png", "BansheeBullet", animation_manager, "sound\\Water1.ogg", 0.4));
	missiles.push_back(new Missile(NULL, { 0, 0 }, 100 / 5, 100 / 5,
		TRUE, L"animation/SkellBossBullet1.png", "SkellBossBullet", animation_manager, "sound\\Explosion1.ogg", 0.4));
	missiles.push_back(new Missile(NULL, { 0, 0 }, 100 / 5, 100 / 5,
		FALSE, L"animation/SkellBossBullet1.png", "SkellBossBullet", animation_manager, "sound\\Explosion1.ogg", 0.8));
	missiles.push_back(new Missile(NULL, { 0, 0 }, w, h,
		FALSE, L"animation/SwordMissile1.png", "SwordMissile", animation_manager, "sound\\Slash8.ogg", 0.4f));
	
	miss_info.reserve(MISSILE_NUM);
}

void MissileManager::Update(const Dungeon* dungeon, AnimationManager* animation_manager)
{
	for (int i = 0; i < missiles.size(); ++i) {
		auto* missile = missiles.at(i);
		missile->Update(animation_manager);
	}
}

void MissileManager::Update(POINT pos[MISSILE_NUM])
{
	for (int i = 0; i < missiles.size(); ++i) {
		if (pos[i].x != -100) {
			missiles[i]->pos = pos[i];

			//printf("pos : %d %d\n", pos[i].x, pos[i].y);

		}
	}
}

void MissileManager::Render(HDC scene_dc, const RECT& bit_rect) const
{
	for (int i = 0; i < miss_info.size(); ++i)
	{
		short id = miss_info[i].id;
		if (id >= 0 && id < missiles.size()) {
			missiles[id]->pos = miss_info[i].pos;
			missiles[id]->looking_direction = miss_info[i].direction;
			missiles[id]->radian = miss_info[i].radian;
			missiles[id]->Render(scene_dc, bit_rect);
		}
	}

	//for (auto* missile : missiles)
		//missile->Render(scene_dc, bit_rect);
}

void MissileManager::Insert(Missile* given_missile)
{
	missiles.push_back(given_missile);
}

void MissileManager::Delete(Missile* given_missile)
{
	for (int i = 0; i < missiles.size(); ++i)
		if (missiles.at(i) == given_missile) {
			delete given_missile;
			missiles.erase(missiles.begin() + i);
			break;
		}
}

void MissileManager::Delete(int id)
{
	auto* missile = missiles.at(id);
	delete missile;
	missiles.erase(missiles.begin() + id);
}
