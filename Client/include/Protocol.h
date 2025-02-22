#pragma once

#define PLAYER_NUM 3
#define MISSILE_NUM 50

enum player_state {
	UNCONNECT, CONNECT, IN_LOBBY, READY, PLAYING, RESULTING
};

enum playing_State {
	DOWN, UP, STANDING, MOVING, DOWNJUMP
};
// Packet ID
constexpr char CS_LOGIN = 1;
constexpr char CS_READY = 2;
constexpr char CS_PLAY = 3;
constexpr char CS_RESULT = 4;

constexpr char SC_LOGIN = 9;
constexpr char SC_READY = 10;
constexpr char SC_ALLREADY = 11;
constexpr char SC_PLAY = 12;

constexpr char SC_MAKE_MONSTER = 13;
constexpr char SC_MONSTER = 14;
constexpr char SC_MISSILE = 15;
constexpr char SC_RESULT = 16;

struct LOGIN_INFO {
	short id;
	in_addr ip;
	short state;
	char name[20];
};

struct CS_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	char name[20];
};

struct CS_READY_PACKET {
	unsigned char size;
	char	type;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;

	short your_id;

	LOGIN_INFO data[PLAYER_NUM];
};

struct SC_READY_PACKET {
	unsigned char size;
	char	type;

	short ready_id;
};

struct SC_RESULT_PACKET {
	unsigned char size;
	char	type;
};

struct PLAYER_MOUSE {
	SHORT left = false;
	SHORT right = false;
	SHORT wheel = false;
	POINT mPos;
	/*int x;
	int y;*/
};
struct PLAYER_KEYBOARD {
	SHORT a = false;
	SHORT s = false;
	SHORT d = false;
	SHORT space = false;
};

struct STATE_PACKET {
	unsigned char size;
	char	type;
};

struct P_STATE_INFO {
	int state=1; // 0 플레이 / 1 결과
	int dungeonID = 0;
};

struct P_STATE {
	unsigned char size;
	char	type;
	P_STATE_INFO info;
};

struct PLAYER_INPUT_INFO {
	short ID;

	struct PLAYER_MOUSE mouse;
	struct PLAYER_KEYBOARD key;

	int time =1;
};

struct PLAYER_INFO {
	short ID;

	POINT PPos;
	POINT MPos;	// Mouse position

	int hp;
	int killMonster;

	bool isMove;			//stand, run
	bool isAttack;			// attack
	bool isMisile;			// Misile
	bool isDash;

	bool Direciton;
};

struct CS_PLAYER_INPUT_INFO_PACKET {
	unsigned char size;
	char	type;

	PLAYER_INPUT_INFO p_info;
};

struct SC_PLAYER_INPUT_INFO_PACKET {
	unsigned char size;
	char	type;
	
	PLAYER_INFO p_info[3];

	int time= 1; // 3��
};

struct PLAYER_INFO_MANAGER { //�������� ���� �� Ŭ��� ������ ������ 
	unsigned char size; 
	char	type;
	short ID;

	POINT PPos;		
	short State;
	//std::string animation_name;			//character sheet name

	int hp;
	int killMonster;

	bool IsMove;			//stand, run
	bool IsAttack;			// attack
	bool IsMisile;			// Misile

	int time = 1; // 3��

	//�����̿� ������ �ʿ��ϴٸ� �߰��� �� 
};

struct MAKE_MONSTER {
	short ID;
	POINT Pos;
	bool Direction;
};

struct SC_MAKE_MONSTER_PACKET {
	unsigned char size;
	char	type;
	MAKE_MONSTER monster[5];
};

struct MONSTER_INFO_MANAGER {
	short id;
	POINT PPos;				

	int hp;

	short player_id;

	bool Direction;			// sight Dir
	//bool isAttack;
	bool isFormerAttack;
	bool isMove;
	bool isStand;
	bool attack_animation;
	short BossAttackID;
};

struct SC_MONSTER_PACKET {
	unsigned char size;
	char	type;

	MONSTER_INFO_MANAGER monster[20];
};

struct MISSILE_INFO {
	short id;
	bool direction;
	float radian;
	POINT pos;
};

struct SC_MISSILE_PACKET {
	unsigned char size;
	char	type;

	MISSILE_INFO info[MISSILE_NUM];
};