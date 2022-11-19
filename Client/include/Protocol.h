#pragma once

struct LOGIN_INFO {
	short	id;
	char player_type;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;

	LOGIN_INFO	data;
};