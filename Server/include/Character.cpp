#include "Character.h"

void Character::ForceGravity(const Dungeon* dungeon)	// ĳ���� ����(LANDING, DOWN, DOWNJUMP, UP)�� ���� ������
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);

	// ĳ���� �� ��ġ�� ����� ��� DOWN���·� �ٲٱ�
	if ((state == State::STANDING || state == State::MOVING || state == State::DOWNJUMP) && MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 255), POINT{ pos.x + width / 2, pos.y + height })) {
		state = State::DOWN;
		MovePos(Direction::DOWN, 5);
	}
	// ���� �� �� ��� �Ǹ� STANDING ���·� �ٲٱ�
	else if (state == State::DOWN && (MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 0), POINT{ pos.x + width / 2, pos.y + height }) || MapPixelCollision(dc_set.buf_dc, RGB(0, 255, 0), POINT{ pos.x + width / 2, pos.y + height }))) {
		state = State::STANDING;
		jump_power = 0;
	}

	if (state == State::STANDING || state == State::MOVING) {	// ������ or DOWN���� �������� �� ĳ���Ͱ� �� �ٷ� ���� ���� �Ϸ� ����
		while (MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 0), POINT{ pos.x + width / 2, pos.y + height - 1 })
			|| MapPixelCollision(dc_set.buf_dc, RGB(0, 255, 0), POINT{ pos.x + width / 2, pos.y + height - 1 })) {
			MovePos(Direction::UP, 1);
		}
	}

	if (state == State::UP) {	// JUMP���� �ö󰡰� �ִ� ���� jump_power�� 0�� �Ǹ� ���°� DOWN���� �ٲ��
		MovePos(Direction::UP, jump_power);
		// jump_power�� ������ �и� Ű��� �߷��� �� �ް� Ƣ������� ������ ��������.
		// �ڿ������� �������� ������� �� �и��� ���� �ִ��� �����Ѵ�. ex) 50.0f : 2000.0f, 25.0f : 4000.0f
		jump_power -= jump_power / 25.0f + dungeon->camera_y_half_range / 4000.0f;
		if (jump_power <= 0) {
			state = State::DOWN;
		}
	}
	else if (state == State::DOWN || state == State::DOWNJUMP) {
		MovePos(Direction::DOWN, jump_power);
		if (jump_power < dungeon->camera_y_half_range / 80.0f) {
			// jump_power�� ������ �и� Ű��� �߷��� ���� ������ ��������.
			// �ڿ������� �������� ���ؼ� ������ �и� 500~1500 ���̷� �����Ѵ�. ���ڴ� �������Դ� �ϱ� ���� �ּ����� ��ġ��.
			// �� ������ �߷� ���� ������ �����ϸ� �� ���� �ٸ� �߷� ������ ������ ���߸ʵ� ������ ������ ���̴�.
			jump_power += jump_power / 500.0f + dungeon->camera_y_half_range / 1000.0f;
		}
		else if (jump_power < dungeon->camera_y_half_range / 40.0f) {
			jump_power += jump_power / 50.0f;
		}
	}
}

void Character::Stand()
{
	state = State::STANDING;
}

void Character::RunLeft()
{
	if (state == State::STANDING)
		state = State::MOVING;

	MovePos(Direction::LEFT, x_move_px);
}

void Character::RunRight()
{
	if (state == State::STANDING)
		state = State::MOVING;

	MovePos(Direction::RIGHT, x_move_px);
}

void Character::DownJump()
{
	state = State::DOWNJUMP;
}

void Character::Jump()
{
	state = State::UP;
	// �и� Ŀ���� �������� ��������.
	jump_power = jump_start_power;
}

bool Character::CanGoLeft(const HDC terrain_dc)
{
	if (CanGoToPos(terrain_dc, POINT{ pos.x - static_cast<LONG>(x_move_px), pos.y + height / 2 }))
		return true;
	return false;
}

bool Character::CanGoRight(const HDC terrain_dc)
{
	if (CanGoToPos(terrain_dc, POINT{ pos.x + width + static_cast<LONG>(x_move_px), pos.y + height / 2 }))
		return true;
	return false;
}

bool Character::CanDownJump(const HDC terrain_dc)
{
	if (CanGoToPos(terrain_dc, POINT{ pos.x,  pos.y + height }))
		return true;
	return false;
}

bool Character::CanJump(const State state)
{
	if (state == State::STANDING || state == State::MOVING)
		return true;
	return false;
}

bool Character::IsOut_Left(const Dungeon* dungeon) const
{
	if (pos.x < 0)
		return true;
	else
		return false;
}

bool Character::IsOut_Right(const Dungeon* dungeon) const
{
	if (pos.x + width > dungeon->dungeon_width)
		return true;
	else
		return false;
}

void Character::NoOut(const Dungeon* dungeon)
{
	if (IsOut_Right(dungeon))
		while (pos.x + width > dungeon->dungeon_width)
			pos.x -= 2;
	else if (IsOut_Left(dungeon))
		while (pos.x < 0)
			pos.x += 2;
}

void Character::MovePos(Direction direction, const int px)
{
	switch (direction) {
	case Direction::LEFT:
		pos.x -= px;
		break;
	case Direction::UP:
		pos.y -= px;
		break;
	case Direction::RIGHT:
		pos.x += px;
		break;
	case Direction::DOWN:
		pos.y += px;
		break;
	}
}

void Character::CS_MovePos(Direction direction, const int px, POINT* PPos)
{
	switch (direction) {
	case Direction::LEFT:
		PPos->x -= px;
		break;
	case Direction::UP:
		PPos->y -= px;
		break;
	case Direction::RIGHT:
		PPos->x += px;
		break;
	case Direction::DOWN:
		PPos->y += px;
		break;
	}
}

void Character::Render(HDC scene_dc, const RECT& bit_rect)
{
	if (looking_direction) {
		image->Draw(scene_dc, pos.x, pos.y, width, height, 0, 0, image->GetWidth(), image->GetHeight());
		if (red_flash_cnt) {
			--red_flash_cnt;
			RedImage(scene_dc, bit_rect, image, pos, width, height, FALSE);
		}
	}
	else {
		DrawFlip(scene_dc, bit_rect, image, pos, width, height);
		if (red_flash_cnt) {
			--red_flash_cnt;
			RedImage(scene_dc, bit_rect, image, pos, width, height, TRUE);
		}
	}
}

void Character::RenderMonsterHP(HDC scene_dc, const RECT& bit_rect) const
{
	int half_size = width / 2;
	int height_size = height / 10;
	POINT center_pos = { pos.x + (width / 2) , pos.y + height + height_size };

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH oldBrush = (HBRUSH)SelectObject(scene_dc, hBrush);
	RECT rect = { center_pos.x - half_size - 1, center_pos.y - 1, center_pos.x + half_size + 1, center_pos.y + height_size + 1 };
	FillRect(scene_dc, &rect, hBrush);
	SelectObject(scene_dc, oldBrush);
	DeleteObject(hBrush);

	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	oldBrush = (HBRUSH)SelectObject(scene_dc, hBrush);
	int hp_max_hp = (static_cast<double>(hp) / static_cast<double>(max_hp)) * (half_size * 2) - half_size;
	rect = { center_pos.x - half_size, center_pos.y, center_pos.x + hp_max_hp , center_pos.y + height_size };
	FillRect(scene_dc, &rect, hBrush);
	SelectObject(scene_dc, oldBrush);
	DeleteObject(hBrush);
}

void Character::RenderPlayerHP(HDC scene_dc, const RECT& bit_rect, const RECT& camera) const
{
	int size = width * 3;
	int height_size = height / 5;
	POINT left_top;
	left_top.x = camera.left;
	left_top.y = camera.top;

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH oldBrush = (HBRUSH)SelectObject(scene_dc, hBrush);
	RECT rect = { left_top.x , left_top.y , left_top.x + size + 1, left_top.y + height_size + 1 };
	FillRect(scene_dc, &rect, hBrush);
	SelectObject(scene_dc, oldBrush);
	DeleteObject(hBrush);

	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	oldBrush = (HBRUSH)SelectObject(scene_dc, hBrush);
	int hp_max_hp = (static_cast<double>(hp) / static_cast<double>(max_hp)) * size;
	rect = { left_top.x + 1, left_top.y + 1, left_top.x + hp_max_hp, left_top.y + height_size };
	FillRect(scene_dc, &rect, hBrush);
	SelectObject(scene_dc, oldBrush);
	DeleteObject(hBrush);
}

void Character::Look(const POINT& target)
{
	if (pos.x < target.x)
		looking_direction = TRUE;
	else
		looking_direction = FALSE;
}

void Character::Look(const Character& target)
{
	if (pos.x < target.pos.x)
		looking_direction = TRUE;
	else
		looking_direction = FALSE;
}

void Character::UpdateInfo(PLAYER_INFO_MANAGER* player)
{

	

	//player->PPos = pos;
	//switch (state) {
	//case  State::DOWN:	player->State = playing_State::DOWN; break;
	//case  State::UP:	player->State = playing_State::UP; break;
	//case  State::STANDING:player->State = playing_State::STANDING; break;
	//case  State::MOVING:player->State = playing_State::MOVING; break;
	//case  State::DOWNJUMP:player->State = playing_State::DOWNJUMP; break;
	//	break;
	//default: player->State = playing_State::STANDING; break;

	//}
	//
	//player->animation_name = animation_name;
	//player->hp = hp;
	//player->IsAttack = is_attacking;
	////player->IsMove =
	
}




void Character::StartAttack(const int given_former_atk_delay, const int given_atk_delay, const RECT& given_atk_rect)
{
	is_attacking = true;
	former_atk_delay = given_former_atk_delay;
	atk_delay = given_atk_delay;
	atk_rect = given_atk_rect;
}

void Character::FinishAttack()
{
	attack_victims.clear();
	is_attacking = false;
}

void Character::AddAttackVictim(const Character* victim)
{
	attack_victims.push_back(victim);
}

bool Character::HasAlreadyAttacked(const Character* victim) const
{
	for (const auto& iter : attack_victims)
		if (iter == victim)
			return true;
	return false;
}