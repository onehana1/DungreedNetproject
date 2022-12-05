#include "Weapon.h"

Weapon::Weapon(const Dungeon* dungeon, const Player* player)
	: width{ dungeon->camera_x_half_range / 3}, height{ dungeon->camera_y_half_range / 2 }
{
	
	//image = start_image = new Image(L"animation/RustyGreatSwordAttack1.png");
	Update(player, {0, 0});
}

Weapon::~Weapon()
{
	if (start_image) {
		delete start_image;
	}
}

void Weapon::Init(const Dungeon* dungeon, const Player* player, const POINT mouse)
{
	width = dungeon->camera_x_half_range / 3;
	height = dungeon->camera_y_half_range / 2;
	Update(player,mouse);
}

void Weapon::Update(const Player* player, const POINT mouse)
{
	pos = player->pos;
	pos.x -= player->width / 4 * 5;
	POINT player_center = { player->pos.x + (player->width / 2), player->pos.y + (player->height / 2) };

	if (player_center.x <=mouse.x) {
		pos.x += player->width / 4;
		looking_direction = TRUE;
	}
	else {
		pos.x -= player->width / 4;
		looking_direction = FALSE;
	}
	if (player_center.y <=mouse.y)
		pos.y += player->height / 8;
	else
		pos.y -= player->height / 8;

	angle = Degree(player_center,mouse);
}




void Weapon::Render(HDC scene_dc, const RECT& bit_rect)
{
	int image_width = image->GetWidth();
	int image_height = image->GetHeight();

	HBITMAP hbm_rotate = RotateImage(scene_dc, image, angle);
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

void Weapon::StartAttack()
{
}

bool Weapon::IsAttackFinished()
{
	if (ATTACT_TIME)
		return true;
	else
		return false;
}