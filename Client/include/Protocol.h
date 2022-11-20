#pragma once

#define PLAYER_NUM 3

// Packet ID
constexpr char CS_LOGIN = 0;

constexpr char SC_LOGIN = 3;

struct LOGIN_INFO {
	short id;
	in_addr ip;
	int state;
	char name[20];
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;

	int id;

	LOGIN_INFO data[PLAYER_NUM];
};

struct CS_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	char name[20];
};