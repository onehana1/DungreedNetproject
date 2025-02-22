#include "Character.h"

void Character::ForceGravity(const Dungeon* dungeon)	// 캐릭터 상태(LANDING, DOWN, DOWNJUMP, UP)에 따른 움직임
{
	InstantDCSet dc_set(RECT{ 0, 0, dungeon->dungeon_width, dungeon->dungeon_height });

	dungeon->dungeon_terrain_image->Draw(dc_set.buf_dc, dc_set.bit_rect);

	// 캐릭터 발 위치가 허공일 경우 DOWN상태로 바꾸기
	if ((state == State::STANDING || state == State::MOVING || state == State::DOWNJUMP) && MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 255), POINT{ pos.x + width / 2, pos.y + height })) {
		state = State::DOWN;
		MovePos(Direction::DOWN, 5);
	}
	// 낙하 중 땅 밟게 되면 STANDING 상태로 바꾸기
	else if (state == State::DOWN && (MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 0), POINT{ pos.x + width / 2, pos.y + height }) || MapPixelCollision(dc_set.buf_dc, RGB(0, 255, 0), POINT{ pos.x + width / 2, pos.y + height }))) {
		state = State::STANDING;
		jump_power = 0;
	}

	if (state == State::STANDING || state == State::MOVING) {	// 오르막 or DOWN에서 착지했을 시 캐릭터가 땅 바로 위에 서게 하려 만듦
		while (MapPixelCollision(dc_set.buf_dc, RGB(255, 0, 0), POINT{ pos.x + width / 2, pos.y + height - 1 })
			|| MapPixelCollision(dc_set.buf_dc, RGB(0, 255, 0), POINT{ pos.x + width / 2, pos.y + height - 1 })) {
			MovePos(Direction::UP, 1);
		}
	}

	if (state == State::UP) {	// JUMP에서 올라가고 있는 상태 jump_power가 0이 되면 상태가 DOWN으로 바뀐다
		MovePos(Direction::UP, jump_power);
		// jump_power를 나누는 분모를 키우면 중력을 덜 받고 튀어오르는 느낌이 강해진다.
		// 자연스러운 움직임을 만드려면 두 분모의 곱은 최대한 유지한다. ex) 50.0f : 2000.0f, 25.0f : 4000.0f
		jump_power -= jump_power / 25.0f + dungeon->camera_y_half_range / 4000.0f;
		if (jump_power <= 0) {
			state = State::DOWN;
		}
	}
	else if (state == State::DOWN || state == State::DOWNJUMP) {
		MovePos(Direction::DOWN, jump_power);
		if (jump_power < dungeon->camera_y_half_range / 80.0f) {
			// jump_power를 나누는 분모를 키우면 중력이 약한 느낌이 강해진다.
			// 자연스러운 움직임을 위해선 후자의 분모를 500~1500 사이로 설정한다. 후자는 떨어지게는 하기 위한 최소한의 수치다.
			// 이 값들을 중력 관련 변수로 설정하면 맵 마다 다른 중력 구현이 가능해 수중맵도 구현이 가능할 것이다.
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
	// 분모가 커지면 점프력이 약해진다.
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

void Character::DrawClipImage(HDC ah_dc, CImage* ap_image, POINT a_clip_start_pos, POINT a_clip_end_pos)
{
	// 부분 이미지의 시작 위치와 폭(cx), 높이(cy) 값을 저장할 변수 선언
	int x, y, cx, cy;
	// X 좌표 결정
	if (a_clip_end_pos.x > a_clip_start_pos.x) {   // 시작 X좌표가 끝 X좌표 보다 작은 경우 ( 정상적인 좌표 )
		x = a_clip_start_pos.x;
		cx = a_clip_end_pos.x - a_clip_start_pos.x;   // 부분 이미지의 폭(width)을 계산
	}
	else {                // 시작 X좌표가 끝 X좌표보다 큰 경우 ( 좌표가 반대로 된 경우 )
		x = a_clip_end_pos.x;
		cx = a_clip_start_pos.x - a_clip_end_pos.x;   // 부분 이미지의 폭(width)을 계산
	}
	// Y 좌표 결정
	if (a_clip_end_pos.y > a_clip_start_pos.y) {    // 시작 Y좌표가 끝 Y좌표 보다 작은 경우 ( 정상적인 좌표 )
		y = a_clip_start_pos.y;
		cy = a_clip_end_pos.y - a_clip_start_pos.y;    // 부분 이미지의 폭(width)을 계산
	}
	else {    // 시작 Y좌표가 끝 Y좌표보다 큰 경우 ( 좌표가 반대로 된 경우 )
		y = a_clip_end_pos.y;
		cy = a_clip_start_pos.y - a_clip_end_pos.y;    // 부분 이미지의 폭(width)을 계산
	}

	// 폭과 높이가 모두 0 이 아니어야지만 출력한다.
	if (cx && cy) 
		// 고정 좌표를 사용하지 않는 경우에는 부분 이미지를 지정할때 사용한 시작좌표에다가 부분 이미지를 출력하고
		// 고정 좌표를 사용하는 경우에는 a_fixed_x, a_fixed_y 에 지정된 좌표 값을 사용하여 출력한다.
		ap_image->Draw(ah_dc, x, y, cx, cy, x, y, cx, cy);
	
}

void Character::ChangeAnimation(const char* a_name)
{
	animation_name = a_name;
	is_animation_load_requested = true;
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

void Character::RenderPlayerHP(HDC scene_dc, const RECT& bit_rect, const RECT& camera) const //여기다 랭킹표 만들거임 
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

	////=================================
	//TCHAR lpOut[100];
	//TCHAR name[50];
	//TCHAR kill[50];
	//TCHAR death[50];
	//HFONT hFont, OldFont;	// 폰트 지정

	//SetBkMode(scene_dc, TRANSPARENT);	// 글자 입력시 배경 투명
	//SetTextColor(scene_dc, RGB(255, 255, 255));	// 글자 하얀색
	//hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
	//	VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));		// 폰트 등 추후 수정
	//OldFont = (HFONT)SelectObject(scene_dc, hFont);

	//for (int i = 0; i < PLAYER_NUM; ++i)
	//{
	//	ZeroMemory(lpOut, 100);
	//	ZeroMemory(kill, 50);
	//	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetName(), strlen(player_list[i]->GetName()), kill, 50); //playerinfo 받는걸로 바꾸기
	//	wsprintf(lpOut, TEXT("킬수여기다가 : %s"), kill);
	//	TextOut(dc_set.buf_dc, 45 + 294, 50 + 10 * i, lpOut, lstrlen(lpOut));

	//	//ZeroMemory(death, 50);
	//	//MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, player_list[i]->GetName(), strlen(player_list[i]->GetName()), death, 50);//playerinfo 받는걸로 바꾸기
	//	//wsprintf(lpOut, TEXT("죽은수여기다가 : %s"), death);
	//	//TextOut(dc_set.buf_dc, 45 + 294 , 50 + 10 * i, lpOut, lstrlen(lpOut));
	//}

	////=================================

	SelectObject(scene_dc, oldBrush);
	DeleteObject(hBrush);

	
}

void Character::RenderPlayerTOP(HDC scene_dc, const RECT& bit_rect, const RECT& camera) 
{


	Ranking[0].Load(L"map_png\\Ranking_Back.png");
	Ranking[1].Load(L"Background\\Crown.png");
	Ranking[0].AlphaBlend(scene_dc, 370, 20, 50);
	//Ranking.StretchBlt(scene_dc, 375, 25, 30, 80); //고쳐야댐 
	Ranking[1].StretchBlt(scene_dc,(int)(pos.x ), (int)(pos.y - 15), 20, 20, 0, 0, 512, 512);//크기를 10x10
	
//순위

	//출력 

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

void Character::UpdateAnimation(AnimationManager* animation_manager)
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