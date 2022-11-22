#pragma once
#pragma once
#include "Common.h"
#include "Protocol.h"
#include "Player.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

std::vector<Player*> player_list(3, NULL);
extern bool game_start;

SOCKET Create_Listen()
{
	int retval;
	//hReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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
	// Ŭ���̾�Ʈ�� ������ ���
	int id = (int)arg;

	while (true) {
		char buf[2];
		DWORD recvByte{ 0 }, recvFlag{ 0 };
		//recv(sock, buf, sizeof(buf), MSG_WAITALL);
		int error_code = recv(player_list[id]->sock, buf, sizeof(buf), 0);
		if (error_code <= 0) {
			closesocket(player_list[id]->sock);

			delete player_list[id];
			player_list[id] = NULL;

			SC_LOGIN_INFO_PACKET my_packet;
			my_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			my_packet.type = SC_LOGIN;
			for (int i = 0; i < player_list.size(); ++i) {
				my_packet.data[i].id = i;
				if (player_list[i] && player_list[i]->GetState() == IN_LOBBY) {
					my_packet.data[i].state = IN_LOBBY;
					my_packet.data[i].ip = player_list[i]->GetIp();
					strcpy(my_packet.data[i].name, player_list[i]->GetName());
				}
				else {
					my_packet.data->state = UNCONNECT;
				}
			}

			for (int i = 0; i < PLAYER_NUM; ++i) {
				my_packet.your_id = i;
				send(player_list[id]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			}

			return 0;
		}

		//if (error_code == SOCKET_ERROR) error_display("RecvSizeType");

		UCHAR size{ static_cast<UCHAR>(buf[0]) };
		UCHAR type{ static_cast<UCHAR>(buf[1]) };
		switch (type)
		{
		case CS_LOGIN: //�α���
		{
			char subBuf[sizeof(char[20])]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);

			printf("��Ŷ ������ :  %d\n", buf[0]);

			char name[20];
			memcpy(&name, &subBuf, sizeof(char[20]));
			
			player_list[id]->SetName(name);
			player_list[id]->SetState(IN_LOBBY);
			
			struct sockaddr_in myaddr;
			int client_len = sizeof(myaddr);
			
			printf("\n[TCP ����] Ŭ���̾�Ʈ �α���: id : %d �г��� : %s\n", id, name);

			getpeername(player_list[id]->sock, (struct sockaddr*)&myaddr, &client_len);

			player_list[id]->SetIp(myaddr.sin_addr);

			SC_LOGIN_INFO_PACKET my_packet;
			my_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
			my_packet.type = SC_LOGIN;
			for (int i = 0; i < player_list.size(); ++i) {
				my_packet.data[i].id = i;
				if (player_list[i] && (player_list[i]->GetState() == IN_LOBBY || player_list[i]->GetState() == READY)) {		// �κ� / ���� �����϶�
					my_packet.data[i].state = player_list[i]->GetState();
					my_packet.data[i].ip = player_list[i]->GetIp();
					strcpy(my_packet.data[i].name, player_list[i]->GetName());
				}
				else {
					my_packet.data[i].state = UNCONNECT;
					printf("%d : UNCONNECT\n", i);
				}
			}

			for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i] && player_list[i]->GetState() >= IN_LOBBY) {
					my_packet.your_id = i;
					send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
			}
			break;
		}
		case CS_READY:
		{
			printf("recv Player %d is Ready\n", id);
			player_list[id]->SetState(READY);
			bool play_start = true;

			for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i] && player_list[i]->GetState() != READY) {
					play_start = false;
					break;
				}
			}

			if (play_start) {
				// �÷��� ���·� ��ȯ
				game_start = true;
				printf("game start\n", id);
			}
			else {
				SC_READY_PACKET my_packet;
				my_packet.size = sizeof(SC_READY_PACKET);
				my_packet.type = SC_READY;
				my_packet.ready_id = id;
				for (int i = 0; i < PLAYER_NUM; ++i) {
					if (player_list[i] && (player_list[i]->GetState() == IN_LOBBY || player_list[i]->GetState() == READY)) {
						send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
				}
				printf("send Player %d is Ready\n", id);
			}
			break;
		}
		case CS_PLAY:
		{
			char subBuf[sizeof(char[20])]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			printf("main game\n");
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			printf("��Ŷ ������ :  %d\n", buf[0]);
			break;

		}

		default:
			printf("Unknown PACKET type [%d]\n", type);
		}
	}
	return 0;
}


