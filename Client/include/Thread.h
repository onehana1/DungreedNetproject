#include "Common.h"
#include "Protocol.h"
#include "Player.h"
#include <vector>
#include <stdio.h>
#define LOCALPORT 9000

extern int g_myid;
std::vector<Player*> player_list(3, NULL);
DWORD WINAPI RecvThread(LPVOID arg);
PLAYER_INPUT_INFO p_input[3];
extern Framework* framework;


DWORD WINAPI RecvThread(LPVOID arg)// //Ŭ���̾�Ʈ���� Recv�� ���� �� �۾��� ����ϴ� �Լ�
{
	SOCKET sock = (SOCKET)arg;
	int my_id = -1;

	while (true) {
		char buf[2];
		DWORD recvByte{ 0 }, recvFlag{ 0 };
		//recv(sock, buf, sizeof(buf), MSG_WAITALL);
		int error_code = recv(sock, buf, sizeof(buf), 0);
		//if (error_code == SOCKET_ERROR) error_display("RecvSizeType");

		UCHAR size{ static_cast<UCHAR>(buf[0]) };
		UCHAR type{ static_cast<UCHAR>(buf[1]) };
		switch (type)
		{
		case SC_LOGIN: //�α���
		{
			char IdBuf[sizeof(short)]{};
			char subBuf[sizeof(LOGIN_INFO[3])]{};
			recv(sock, IdBuf, sizeof(IdBuf), 0);
			recv(sock, subBuf, sizeof(subBuf), 0);

			short id;
			LOGIN_INFO loginInfo[3];
			memcpy(&id, &IdBuf, sizeof(short));
			memcpy(&loginInfo, &subBuf, sizeof(LOGIN_INFO[3]));
			g_myid = my_id = id;

			printf("id : %d\n", my_id);

			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (!player_list[i]) {
					player_list[i] = new Player;
				}
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &loginInfo[i].ip, addr, sizeof(addr));
				printf("ip : %s\n", addr);
				player_list[i]->SetIp(addr);
				player_list[i]->SetName(loginInfo[i].name);
				player_list[i]->SetState(loginInfo[i].state);

				printf("name : %s	state : %d\n", player_list[i]->GetName(), player_list[i]->GetState());
			}
			break;
		}
		case SC_READY:
		{
			char IdBuf[sizeof(short)]{};
			recv(sock, IdBuf, sizeof(IdBuf), 0);

			short id;
			memcpy(&id, &IdBuf, sizeof(short));
			player_list[id]->SetState(READY);
				

			break;
		}
		case SC_ALLREADY:
			framework->ChangeScene(3);
			break;

			
		case SC_PLAY:
		{
			printf("playing\n");
			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (player_list[i])
					player_list[i]->SetState(PLAYING);
			}
			char PlayBuf[sizeof(SC_PLAYER_INPUT_INFO_PACKET[3])]{};
			recv(sock, PlayBuf, sizeof(PlayBuf), 0);
			printf("packet size :  %d\n", buf[0]);

			PLAYER_INPUT_INFO temp[3];

			memcpy(&temp, &PlayBuf, sizeof(PLAYER_INPUT_INFO[3]));

			
			
			for (int i = 0; i < 3; i++) {
				if (temp[i].time !=0) {
					p_input[temp[i].ID].ID = temp[i].ID;

					p_input[temp[i].ID].mouse.right = temp[i].mouse.right;
					p_input[temp[i].ID].mouse.left = temp[i].mouse.left;
					p_input[temp[i].ID].mouse.wheel = temp[i].mouse.wheel;
					p_input[temp[i].ID].mouse.mPos.x = temp[i].mouse.mPos.x;
					p_input[temp[i].ID].mouse.mPos.y = temp[i].mouse.mPos.y;

					p_input[temp[i].ID].key.a = temp[i].key.a;
					p_input[temp[i].ID].key.s = temp[i].key.s;
					p_input[temp[i].ID].key.d = temp[i].key.d;
					p_input[temp[i].ID].key.space = temp[i].key.space;

					p_input[temp[i].ID].time = temp[i].time;
				}

			}

			
			break;
		}

		case SC_RESULT:
		{
			printf("result\n");
			char PlayBuf[sizeof(P_STATE[3])]{}; // 잡은 몬스터와 죽은 횟수로 수정 필요
			recv(sock, PlayBuf, sizeof(PlayBuf), 0);
			printf("packet size :  %d\n", buf[0]);

			P_STATE temp[3];
			memcpy(&temp, &PlayBuf, sizeof(P_STATE[3]));

			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (player_list[i])
					player_list[i]->SetState(RESULTING);
			}


			/*char IdBuf[sizeof(short)]{};
			recv(sock, IdBuf, sizeof(IdBuf), 0);

			short id;
			memcpy(&id, &IdBuf, sizeof(short));
			player_list[id]->SetState(RESULTING);*/

			break;
		}
		
		case SC_MAKE_MONSTER:
		{
			char Buf[2];
			recv(sock, Buf, sizeof(Buf), 0);	// 2씩 더 받아지는 오류 대처.... 

			char subBuf[sizeof(MAKE_MONSTER[5])]{};
			recv(sock, subBuf, sizeof(subBuf), 0);

			MAKE_MONSTER monsterInfo[5];
			memcpy(&monsterInfo, &subBuf, sizeof(MAKE_MONSTER[5]));

			for (int i = 0; i < 5; ++i) {
				printf("%d monster : %d (%d, %d) %d\n", i, monsterInfo[i].ID, monsterInfo[i].Pos.x, monsterInfo[i].Pos.y, monsterInfo[i].Direction);
			}

			framework->GetPlayScene()->GetMonsterManager()->Appear(monsterInfo);
			break;
		}
		default:
			printf("Unknown PACKET type [%d]\n", type);
		}
	}
}

/*
DWORD WINAPI ClientThread(SOCKET client_sock) //Ŭ�� ������ ��Ŵ��
{
	char buf[2];
	WSABUF wsabuf{ sizeof(buf), buf };
	DWORD recvByte{ 0 }, recvFlag{ 0 };
	//recv(sock, buf, sizeof(buf), MSG_WAITALL);
	int error_code = WSARecv(sock, &wsabuf, 1, &recvByte, &recvFlag, nullptr, nullptr);
	//if (error_code == SOCKET_ERROR) error_display("RecvSizeType");

	UCHAR size{ static_cast<UCHAR>(buf[0]) };
	UCHAR type{ static_cast<UCHAR>(buf[1]) };
	switch (type)
	{
	case SC_LOGIN_INFO: //�α���
	{
		char subBuf[sizeof(LOGIN_INFO)]{};
		WSABUF wsabuf{ sizeof(subBuf), subBuf };
		DWORD recvByte{}, recvFlag{};
		WSARecv(sock, &wsabuf, 1, &recvByte, &recvFlag, nullptr, nullptr);

		LOGIN_INFO loginInfo;
		memcpy(&loginInfo, &subBuf, sizeof(LOGIN_INFO));
		g_myid = loginInfo.id;
		player_type = loginInfo.player_type;
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", type);
	}
}
/*
char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char* argv[])
{
	int retval;

	// ������ �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE];
	const char* testdata[] = {
		"�ȳ��ϼ���",
		"�ݰ�����",
		"���õ��� �� �̾߱Ⱑ ���� �� ���׿�",
		"���� �׷��׿�",
	};
	int len;

	// ������ ������ ���
	for (int i = 0; i < 4; i++) {
		// ������ �Է�(�ùķ��̼�)
		len = (int)strlen(testdata[i]);
		strncpy(buf, testdata[i], len);
		buf[len++] = '\n';

		// ������ ������
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);
	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
*/