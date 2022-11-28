#pragma once

#define PLAYER_NUM 3

enum player_state {
	UNCONNECT, CONNECT, IN_LOBBY, READY, PLAYING
};

// Packet ID
constexpr char CS_LOGIN = 1;
constexpr char CS_READY = 2;
constexpr char CS_PLAY = 3;


constexpr char SC_LOGIN = 10;
constexpr char SC_READY = 11;
constexpr char SC_PLAY = 12;


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

struct SC_READY_PACKET {
	unsigned char size;
	char	type;

	short ready_id;
};

struct CS_READY_PACKET {
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

struct PLAYER_INPUT_INFO {
	unsigned char size;
	char	type;
	short ID;

	struct PLAYER_MOUSE mouse;
	struct PLAYER_KEYBOARD key;

	int time =1; // 3√ 
};

struct CS_PLAYER_INPUT_INFO_PACKET {
	unsigned char size;
	char	type;
	short ID; //

	struct PLAYER_MOUSE mouse;
	struct PLAYER_KEYBOARD key;	

};

struct SC_PLAYER_INPUT_INFO_PACKET {
	unsigned char size;
	char	type;
	short ID; //

	struct PLAYER_MOUSE mouse;
	struct PLAYER_KEYBOARD key;

	int time= 1; // 3√ 

};

struct PLAYER_INFO_MANAGER {
	POINT PPos;				//State
	int hp;
	int killMonster;

	bool Direction;			//MOUSE - Left(0), Right(1)
	bool IsMove;			//stand, run
};