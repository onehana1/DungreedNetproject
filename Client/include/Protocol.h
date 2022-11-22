#pragma once

#define PLAYER_NUM 3

enum player_state {
	UNCONNECT, CONNECT, READY
};


// Packet ID
constexpr char CS_LOGIN = 1;
constexpr char CS_PLAY = 2;


constexpr char SC_LOGIN = 3;

struct LOGIN_INFO {
	short id;
	in_addr ip;
	short state;
	char name[20];
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;

	short your_id;

	LOGIN_INFO data[PLAYER_NUM];
};

struct CS_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	char name[20];
};

struct PLAYER_MOUSE {
	bool left;
	bool right;
	bool wheel;
	POINT mPos;
	/*int x;
	int y;*/
};
struct PLAYER_KEYBOARD {
	bool a;
	bool s;
	bool d;
	bool space;
};

struct CS_PLAYER_INPUT_INFO_PACKET {
	unsigned char size;
	char	type;
	char name[20];
	short ID; //

	struct PLAYER_MOUSE mouse;
	struct PLAYER_KEYBOARD key;

	

};
struct PLAYER_INFO_MANAGER {
	POINT PPos;				//State
	int hp;
	int killMonster;

	bool Direction;			//MOUSE - Left(0), Right(1)
	bool IsMove;			//stand, run
	
};