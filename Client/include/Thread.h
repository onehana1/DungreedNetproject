#include "Common.h"
#include "Protocol.h"
#include "Player.h"
#include <vector>
#include <stdio.h>
#define LOCALPORT 9000

std::vector<Player*> player_list(3, NULL);
DWORD WINAPI RecvThread(LPVOID arg);

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
			my_id = id;

			TCHAR s[80] = _T("Debug : ���� SC_LOGI");
			OutputDebugString(s);

			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (!player_list[i]) {
					player_list[i] = new Player;
				}
				player_list[i]->SetIp(loginInfo[i].ip);
				player_list[i]->SetName(loginInfo[i].name);
				player_list[i]->SetState(loginInfo[i].state);
			}
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

	// ����� �μ��� ������ IP �ּҷ� ���
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