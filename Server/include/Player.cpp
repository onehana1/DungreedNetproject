#include "Player.h"

void Player::PlaceWithDungeonLeft(const Dungeon* dungeon)
{
	pos = dungeon->left_start_pos;
}

void Player::PlaceWithDungeonRight(const Dungeon* dungeon)
{
	pos = dungeon->right_start_pos;
}

void Player::Init(const Dungeon* dungeon



)
{
	state = State::DOWN;
	x_move_px = dungeon->camera_x_half_range / 60.0f;
	jump_start_power = dungeon->camera_y_half_range / 32.0f;
	jump_power = 0;
	dash_power = 0;
	dash_radian = 0;
	width = dungeon->camera_x_half_range / PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE;
	height = dungeon->camera_x_half_range / PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE;
}

void Player::Update(const Dungeon* dungeon, Weapon* weapon, MissileManager* missile_manager     )
{	
	if (dash_power <= 0) {
		KeyProc(dungeon, missile_manager);
		AttackProc(weapon,missile_manager );
		ForceGravity(dungeon);
		ForceGravity(dungeon);
		Look(mouse);
	}
	DashProc(Degree(mouse, pos), dungeon, dungeon->camera_x_half_range / 16  );

}

void Player::KeyProc(const Dungeon* dungeon, MissileManager* missile_manager



)
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);

	if (state == State::MOVING && !GetAsyncKeyState('A') && !GetAsyncKeyState('D') && !GetAsyncKeyState('S') && !GetAsyncKeyState(VK_SPACE)) {
		Stand();
		return;
	}

	if (GetAsyncKeyState('A'))
		if (CanGoLeft(dc_set.buf_dc))
			RunLeft();

	if (GetAsyncKeyState('D'))
		if (CanGoRight(dc_set.buf_dc))
			RunRight();

	if ((GetAsyncKeyState('S')) && (GetAsyncKeyState(VK_SPACE))) {
		if (CanDownJump(dc_set.buf_dc))
			DownJump();
	}
	else if (GetAsyncKeyState(VK_SPACE))
		if (CanJump(state)) {
			//("sound\\jump.mp3");
			Jump();
		}

	if (IsOut_Left(dungeon) && !dungeon->CanGoPrev())
		MovePos(Direction::RIGHT, x_move_px);
	else if (IsOut_Right(dungeon) && !dungeon->CanGoNext())
		MovePos(Direction::LEFT, x_move_px);
}

void Player::DashProc(float radian, const Dungeon* dungeon, const int px )
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);

	if (dash_power == 0 && GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
		dash_power = dungeon->camera_y_half_range / 24.0f;
		dash_radian = radian;
		//Play("sound\\dash.mp3");
	}

	if (dash_power > 0) {
		POINT old_pos = pos;

		if (looking_direction) {
			POINT desti_pos = { pos.x + px * cos(dash_radian), pos.y - px * sin(dash_radian)};
			POINT desti_character_foot_pos = { desti_pos.x + width / 2, desti_pos.y + height / 5 * 4 };
			if (CanGoToPos(dc_set.buf_dc, desti_character_foot_pos))
				pos = desti_pos;
		}
		else {
			POINT desti_pos = { pos.x - px * cos(dash_radian), pos.y - px * sin(dash_radian)};
			POINT desti_character_foot_pos = { desti_pos.x + width / 2, desti_pos.y + height / 5 * 4 };
			if (CanGoToPos(dc_set.buf_dc, desti_character_foot_pos))
				pos = desti_pos;
		}
		dash_power -= 1;

		if (dash_power < 0) {
			dash_power = -50;
			state = State::DOWN;
		}

		if (IsOut_Left(dungeon) && !dungeon->CanGoPrev())
			pos = old_pos;
		else if (IsOut_Right(dungeon) && !dungeon->CanGoNext())
			pos = old_pos;

	}

	if (dash_power < 0) {
		dash_power++;
		if (dash_power >= 0) {
			dash_power = 0;
		}
	}
}

void Player::AttackProc(Weapon* weapon, MissileManager* missile_manager   )
{
	if (is_attacking || atk_delay) {
		if (is_attacking) {

			if (is_doing_missile_attack && former_atk_delay == 0) {
				float radian = Degree(mouse, pos);

				if (looking_direction)
					missile_manager->Insert(new Missile(this, pos, width, height / 2, radian, x_move_px * 2, 300, TRUE, 3, 70));
				else
					missile_manager->Insert(new Missile(this, pos, width, height / 2, radian, x_move_px * 2, 300, FALSE, 3, 70));
			}
			else if (!is_doing_missile_attack) {
				int atk_rect_center_x;
				int atk_rect_center_y;
				if (looking_direction) {
					atk_rect_center_x = pos.x + width * 1.5 * cos(atk_radian);
				}
				else {
					atk_rect_center_x = pos.x - width * 0.5 * cos(atk_radian);
				}

				atk_rect_center_y = pos.y + height / 2 - width * sin(atk_radian);

				atk_rect = RECT{ atk_rect_center_x - width / 2, atk_rect_center_y - height / 2, atk_rect_center_x + width / 2, atk_rect_center_y + height / 2 };
			}

			--former_atk_delay;

			if (weapon->IsAttackFinished())
				FinishAttack();
		}
		else 
			--atk_delay;
	}
	else if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) {
		//sound_manager->Play("sound\\swing1.mp3");
		weapon->StartAttack();

		is_doing_missile_attack = true;
		
		StartAttack(10, 20, RECT{});
	}
	else if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		//sound_manager->Play("sound\\swing1.mp3");
		weapon->StartAttack();
		atk_radian = Degree(mouse, pos);

		is_doing_missile_attack = false;

		StartAttack(10, 3, RECT{  });
	}
}
