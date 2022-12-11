#pragma once
#pragma once
#include "Common.h"
#include "Player.h"
#include "Scene.h"
#include "Protocol.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

std::vector<Player*> player_list(3, NULL);
//Scene* Info_Scene = new Scene;
extern Scene* scene;
extern bool game_start;


int StartDun = 0;
int StartChange = 0;
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

				STATE_PACKET my_packet;
				my_packet.size = sizeof(STATE_PACKET);
				my_packet.type = SC_ALLREADY;

				for (int i = 0; i < PLAYER_NUM; ++i) {
					if (player_list[i]&& player_list[i]->GetState() == READY) {
						send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					}
				}
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
				printf("Ÿ�̸� ��\n");
				EndTime = (unsigned)time(NULL) + 5;
				StartDun = 1;
			}
			StartTime = (unsigned)time(NULL);
			CntTime = EndTime - StartTime;

			//printf("main game\n");
			player_list[id]->SetState(PLAYING);
			 
			char Buf[2]{};
			recv(player_list[id]->sock, Buf, sizeof(Buf), 0);	//padding

			char subBuf[sizeof(PLAYER_INPUT_INFO)]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			
			PLAYER_INPUT_INFO p_input;
			memcpy(&p_input, &subBuf, sizeof(PLAYER_INPUT_INFO));
			p_input.ID = id;

			if (scene) {
				scene->InputUpdate(p_input);
			}
			else {
				printf("scene null\n");
			}
			
			//printf("input update");
			
			SC_RESULT_PACKET  my_packet{};
			my_packet.size = sizeof(SC_RESULT_PACKET);
			if (CntTime < 0) {
				StartChange = 0;
				my_packet.type = SC_RESULT;
				for (int i = 0; i < PLAYER_NUM; ++i) {
					if (player_list[i]) {
						send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
						//iD�� �����Ǵ� ���� ������ ����.. 
					}
				}
			}

			

			//else
				//my_packet.type = SC_PLAY;
			/*
			printf("%d id ����ϱ�\n", id);
			my_packet.ID = id;

			my_packet.PPos = scene->SC_INFO[id].PPos;
			my_packet.State= scene->SC_INFO[id].State;
			//my_packet.animation_name = scene->SC_INFO[id].animation_name;

			my_packet.hp = scene->SC_INFO[id].hp;
			my_packet.killMonster = scene->SC_INFO[id].killMonster;

			my_packet.IsMove = scene->SC_INFO[id].IsMove;
			my_packet.IsAttack = scene->SC_INFO[id].IsAttack;
			my_packet.IsMisile = scene->SC_INFO[id].IsMisile;

			printf("%d �����߽��ϴ�\n", id);
			//scene->SC_INFO[id];

			//SC_INFO[ID]�� ���� ���� ã�� 3�������� ��Ѹ��� 
			for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i]) {
					send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
					//iD�� �����Ǵ� ���� ������ ����.. 
				}
			}

			printf("cnttime : %d\n", my_packet.time);
			*/


			

			/*for (int i = 0; i < PLAYER_NUM; ++i) {
				if (player_list[i] && (player_list[i]->GetState() == PLAYING)) {
					send(player_list[i]->sock, subBuf, sizeof(subBuf), NULL);
				}
			}*/

			break;

		}
		case CS_RESULT:
		{
			char Buf[2]{};
			recv(player_list[id]->sock, Buf, sizeof(Buf), 0);	//padding
			//printf("result\n");
			player_list[id]->SetState(RESULTING);

			char subBuf[sizeof(P_STATE_INFO)]{};
			recv(player_list[id]->sock, subBuf, sizeof(subBuf), 0);
			//printf("��Ŷ ������ :  %d\n", buf[0]);


			//�÷��̾�鿡�� killmonster�̶� ���� �� �����ֱ� - ���߿� ���� �κ� ���� �����°ɷ� ����
			//P_STATE  my_packet{};
			//my_packet.size = sizeof(P_STATE);
			//my_packet.type = SC_RESULT;
			//my_packet.state = 0;
			//for (int i = 0; i < PLAYER_NUM; ++i) {
			//	if (player_list[i]) {
			//		send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
			//	}
			//}

			//�ð� �ְ� �ٽ� ���(����ȯ�ø��� ȣ��) 
			int dungeonID = 0;
			if (StartChange== 0) {
				printf("-----�ð� �ٽ� ��-----------\n");
				EndTime = (unsigned)time(NULL) + 5;
				StartChange = 1;
			}
			StartTime = (unsigned)time(NULL);
			CntTime = EndTime - StartTime;
			//printf("�� - %d", CntTime);

				
			if (CntTime < 0) { //���-.�÷��̾� 
				printf("send sc_play!\n");
				scene->Check_Dun_Change[id] = true;
				P_STATE  my_packet{};
				my_packet.size = sizeof(P_STATE);
				my_packet.type = SC_RESULT;
				my_packet.info.state = 0;
				my_packet.info.dungeonID = scene->GetDungeon()->next_dungeon_id;
				StartChange = 0;
				for (int i = 0; i < PLAYER_NUM; ++i) {
					if (player_list[i]) {
						send(player_list[i]->sock, reinterpret_cast<char*>(&my_packet), sizeof(my_packet), NULL);
						player_list[id]->SetState(PLAYING);
						//iD�� �����Ǵ� ���� ������ ����.. 
					}
				}

			}



		}

		default:
			printf("Unknown PACKET type [%d]\n", type); //���� üũ �Ⱥ����� ��� ����
			//break;
		}
	}
	return 0;
}


