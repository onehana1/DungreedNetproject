#include "Player.h"

void Player::PlaceWithDungeonLeft(const Dungeon* dungeon)
{
	pos = dungeon->left_start_pos;
}

void Player::PlaceWithDungeonRight(const Dungeon* dungeon)
{
	pos = dungeon->right_start_pos;
}

void Player::Init(const Dungeon* dungeon)
{
	state = State::DOWN;
	x_move_px = dungeon->camera_x_half_range / 60.0f;
	jump_start_power = dungeon->camera_y_half_range / 32.0f;
	jump_power = 0;
	dash_power = 0;
	dash_radian = 0;
	width = dungeon->camera_x_half_range / PLAYER_WIDTH_PER_CAMERA_X_HALF_RANGE;
	height = dungeon->camera_x_half_range / PLAYER_HEIGHT_PER_CAMERA_Y_HALF_RANGE;

	pos.x = 500; pos.y = 100;
}

void Player::Update(const Dungeon* dungeon, Weapon* weapon, MissileManager* missile_manager )
{	
	if (dash_power <= 0) {
		if (have_to_update) {
			have_to_update = false;
			KeyProc(dungeon, missile_manager);
		}
		AttackProc(weapon,missile_manager);
		ForceGravity(dungeon);
		ForceGravity(dungeon);
		Look(input.mouse.mPos);
	}
	DashProc(Degree(input.mouse.mPos, pos), dungeon, dungeon->camera_x_half_range / 16  );

}

void Player::SC_Update(const Dungeon* dungeon, PLAYER_MOUSE mouse, PLAYER_KEYBOARD key, POINT* Ppos)
{
	if (!mouse.left) {
		SC_KeyProc(dungeon, key, Ppos);
	}
	
}//

void Player::SC_Update2(const Dungeon* dungeon, Weapon* weapon, MissileManager* missile_manager, PLAYER_KEYBOARD key, PLAYER_MOUSE mouse)//
{
	if (dash_power <= 0) {
		SC_KeyProc(dungeon, key, &pos);						//좌표갱신
		SC_AttackProc(weapon, missile_manager, key,mouse);	//어택에 따른 상태, 미사일 위치, 갱신 -> 관리되는 몬스터들과의 채킹 필요 
		ForceGravity(dungeon);
		ForceGravity(dungeon);								//? 왜두개 
		Look(mouse.mPos);									//마우스에 따른 방향 설정 -> AttackProc에서 사용 
	}
	//DashProc(Degree(mouse.mPos, pos), dungeon, dungeon->camera_x_half_range / 16);
}

void Player::SetInput(PLAYER_INPUT_INFO p_input)
{
	input.key.a = p_input.key.a; 
	input.key.s = p_input.key.s;
	input.key.d = p_input.key.d;
	input.key.space = p_input.key.space;

	input.mouse.mPos = p_input.mouse.mPos;
	input.mouse.left = p_input.mouse.left;
	input.mouse.right = p_input.mouse.right;
	input.mouse.wheel = p_input.mouse.wheel;
	
	have_to_update = true;
}

bool Player::isMove()
{
	if (state == State::STANDING || state == State::DOWN) {
		return false;
	}
	else {
		return true;
	}
}

void Player::KeyProc(const Dungeon* dungeon, MissileManager* missile_manager)
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);

	if (state == State::MOVING && !input.key.a && !input.key.d && !input.key.s && !input.key.space) {
		Stand();
		return;
	}

	if (input.key.a)
		if (CanGoLeft(dc_set.buf_dc))
			RunLeft();

	if (input.key.d)
		if (CanGoRight(dc_set.buf_dc))
			RunRight();

	if (input.key.s && input.key.space) {
		if (CanDownJump(dc_set.buf_dc))
			DownJump();
	}
	else if (input.key.space)
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

	if (dash_power == 0 && input.mouse.right) {
		input.mouse.right = false;
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
	else if (input.mouse.wheel) {
		//sound_manager->Play("sound\\swing1.mp3");
		input.mouse.wheel = false;
		weapon->StartAttack();
		isMisile = true;
		is_doing_missile_attack = true;
		
		StartAttack(10, 20, RECT{});
	}
	else if (input.mouse.left) {
		input.mouse.left = false;
		//sound_manager->Play("sound\\swing1.mp3");
		weapon->StartAttack();
		atk_radian = Degree(mouse, pos);
		isAttack = true;
		is_doing_missile_attack = false;

		StartAttack(10, 3, RECT{  });
	}
}

void Player::SC_KeyProc(const Dungeon* dungeon, PLAYER_KEYBOARD key, POINT* Ppos)
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);
	if (state == State::MOVING && !key.a && !key.d && !key.s && !key.space) {
		Stand();
		return;
	}

	if (key.a)
		if (CanGoLeft(dc_set.buf_dc))
			RunLeft();

	if (key.d)
		if (CanGoRight(dc_set.buf_dc))
			RunRight();

	if ((key.s) && (key.space)) {
		if (CanDownJump(dc_set.buf_dc))
			DownJump();
	}
	else if (key.space)
		if (CanJump(state)) {
			//("sound\\jump.mp3");
			Jump();
		}

	if (IsOut_Left(dungeon) && !dungeon->CanGoPrev())
		CS_MovePos(Direction::RIGHT, x_move_px, Ppos);
	else if (IsOut_Right(dungeon) && !dungeon->CanGoNext())
		CS_MovePos(Direction::LEFT, x_move_px, Ppos);
}

void Player::SC_DashProc(float radian, const Dungeon* dungeon, int* px)
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);

	if (dash_power == 0 && input.mouse.right) {
		dash_power = dungeon->camera_y_half_range / 24.0f;
		dash_radian = radian;
		isDash = true;
		//Play("sound\\dash.mp3");
	}

	if (dash_power > 0) {
		POINT old_pos = pos;

		if (looking_direction) {
			POINT desti_pos = { pos.x + *px * cos(dash_radian), pos.y - *px * sin(dash_radian) };
			POINT desti_character_foot_pos = { desti_pos.x + width / 2, desti_pos.y + height / 5 * 4 };
			if (CanGoToPos(dc_set.buf_dc, desti_character_foot_pos))
				pos = desti_pos;
		}
		else {
			POINT desti_pos = { pos.x - *px * cos(dash_radian), pos.y - *px * sin(dash_radian) };
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

void Player::SC_AttackProc(Weapon* weapon, MissileManager* missile_manager, PLAYER_KEYBOARD key, PLAYER_MOUSE mouse)
{

	if (is_attacking || atk_delay) {
		if (is_attacking) {

			if (is_doing_missile_attack && former_atk_delay == 0) {
				float radian = Degree(mouse.mPos, pos);

				if (looking_direction) //미사일 업뎃장소, 보내는 거에 미사일도 필요할듯. 관리되고있는 missile vector값을 보내자.  
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
	else if (mouse.wheel) {
		//sound_manager->Play("sound\\swing1.mp3");
		weapon->StartAttack();

		is_doing_missile_attack = true;

		StartAttack(10, 20, RECT{});
	}
	else if (mouse.left) {
		//sound_manager->Play("sound\\swing1.mp3");
		weapon->StartAttack();
		atk_radian = Degree(mouse.mPos, pos);

		is_doing_missile_attack = false;

		StartAttack(10, 3, RECT{  });
	}
}


