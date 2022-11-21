#pragma once
#pragma once
#include "Common.h"
#include "Protocol.h"
#include "Player.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

std::vector<Player*> player_list(3, NULL);

SOCKET Create_Listen()
{
	int retval;
	//hReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	return listen_sock;
}

DWORD WINAPI ClientThread(LPVOID arg)
{
	// 클라이언트와 데이터 통신
	int id = (int)arg;

	while (true) {
		char buf[2];
		DWORD recvByte{ 0 }, recvFlag{ 0 };
		//recv(sock, buf, sizeof(buf), MSG_WAITALL);
		int error_code = recv(player_list[id]->sock, buf, sizeof(buf), 0);
		//if (error_code == SOCKET_ERROR) error_display("RecvSizeType");

		UCHAR size{ static_cast<UCHAR>(buf[0]) };
		UCHAR type{ static_cast<UCHAR>(buf[1]) };
		switch (type)
		{
		case CS_LOGIN: //로그인
		{
			char subBuf[sizeof(char[20])]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);

			printf("패킷 사이즈 :  %d\n", buf[0]);

			char name[20];
			memcpy(&name, &subBuf, sizeof(char[20]));
			
			player_list[id]->SetName(name);
			int client_len;
			struct sockaddr_in myaddr;
			
			printf("\n[TCP 서버] 클라이언트 로그인: id : %d 닉네임 : %s\n", id, name);

			getpeername(player_list[id]->sock, (struct sockaddr*)&myaddr, &client_len);

			player_list[id]->SetIp(myaddr.sin_addr);

			SC_LOGIN_INFO_PACKET my_packet;
			my_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			my_packet.type = SC_LOGIN;
			for (int i = 0; i < player_list.size(); ++i) {
				my_packet.data->id = i;
				if (player_list[id] && player_list[id]->GetState() == CONNECT) {
					my_packet.data->state = CONNECT;
					my_packet.data->ip = player_list[id]->GetIp();
					char* n = player_list[i]->GetName();
					//strcpy(my_packet.data->name, n);
				}
				else {
					my_packet.data->state = UNCONNECT;
				}
			}
			send(player_list[id]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);

			break;
		}
		case CS_PLAY:
		{
			char subBuf[sizeof(char[20])]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			printf("main game\n");
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			printf("패킷 사이즈 :  %d\n", buf[0]);


		}

		default:
			printf("Unknown PACKET type [%d]\n", type);
		}
	}
	return 0;
}


