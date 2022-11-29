#pragma once
#pragma once
#include "Common.h"
#include "Player.h"
#include "Protocol.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

std::vector<Player*> player_list(3, NULL);
extern bool game_start;


int StartDun = 0;
int StartTime;
int EndTime;

int StartRTime;
int EndRTime;

int CntTime;
int CntRTime;


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
				if (player_list[i] && (player_list[i]->GetState() == IN_LOBBY || player_list[i]->GetState() == READY)) {
					my_packet.data[i].state = IN_LOBBY;
					my_packet.data[i].ip = player_list[i]->GetIp();
					strcpy(my_packet.data[i].name, player_list[i]->GetName());
				}
				else {
					my_packet.data->state = UNCONNECT;
				}
			}

			printf("%d player ����\n", id);
			
			for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i] && (player_list[i]->GetState() == IN_LOBBY || player_list[i]->GetState() == READY)) {
					my_packet.your_id = i;
					send(player_list[id]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
			}
			printf("�÷��̾� %d�� ���� ����\n", id);
			return 0;
		}

		//if (error_code == SOCKET_ERROR) error_display("RecvSizeType");

		UCHAR size{ static_cast<UCHAR>(buf[0]) };
		UCHAR type{ static_cast<UCHAR>(buf[1]) };
		//SHORT key{ static_cast<SHORT>(buf[2]) };

		

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

			//�ð�
			if (StartDun == 0) {
				EndTime = (unsigned)time(NULL) + 3;
				StartDun = 1;
			}
			StartTime = (unsigned)time(NULL);
			CntTime = EndTime - StartTime;

			
			//

			printf("main game\n");
			player_list[id]->SetState(PLAYING);

			char subBuf[sizeof(CS_PLAYER_INPUT_INFO_PACKET)]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			printf("��Ŷ ������ :  %d\n", buf[0]);
			
			PLAYER_INPUT_INFO p_input;
			memcpy(&p_input, &subBuf, sizeof(CS_PLAYER_INPUT_INFO_PACKET));

			
			SC_PLAYER_INPUT_INFO_PACKET  my_packet{};
			my_packet.size = sizeof(SC_PLAYER_INPUT_INFO_PACKET);
			if (CntTime < 0) {
				my_packet.type = SC_RESULT;
				StartDun = 0;
			}
			else
				my_packet.type = SC_PLAY;
			my_packet.ID = id;

			my_packet.mouse.right = p_input.mouse.right;
			my_packet.mouse.left = p_input.mouse.left;
			my_packet.mouse.wheel = p_input.mouse.wheel;
			my_packet.mouse.mPos.x = p_input.mouse.mPos.x;
			my_packet.mouse.mPos.y = p_input.mouse.mPos.y;

			my_packet.key.a = p_input.key.a;
			my_packet.key.s = p_input.key.s;
			my_packet.key.d = p_input.key.d;
			my_packet.key.space = p_input.key.space;

			my_packet.time = CntTime;

			for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i]) {
					send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
				}
			}

			printf("cnttime : %d\n", my_packet.time);



			

			/*for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i] && (player_list[i]->GetState() == PLAYING)) {
					send(player_list[i]->sock, subBuf, sizeof(subBuf), NULL);
				}
			}*/

			break;

		}
		case CS_RESULT:
		{
			printf("rsult\n");
			char subBuf[sizeof(char[20])]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			printf("��Ŷ ������ :  %d\n", buf[0]);

			//�ð� �ְ� �ٽ� ���

		}

		default:
			printf("Unknown PACKET type [%d]\n", type);
		}
	}
	return 0;
}


