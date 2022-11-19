#include "Common.h"
#include "Protocol.h"
#define LOCALPORT 9000



DWORD WINAPI RecvThread(SOCKET client_sock)// //클라이언트에서 Recv와 수신 후 작업을 담당하는 함수
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
	case SC_LOGIN_INFO: //로그인
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


DWORD WINAPI ClientThread(SOCKET client_sock) //클라 스레드 통신담당
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
	case SC_LOGIN_INFO: //로그인
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

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char* argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	const char* testdata[] = {
		"안녕하세요",
		"반가워요",
		"오늘따라 할 이야기가 많을 것 같네요",
		"저도 그렇네요",
	};
	int len;

	// 서버와 데이터 통신
	for (int i = 0; i < 4; i++) {
		// 데이터 입력(시뮬레이션)
		len = (int)strlen(testdata[i]);
		strncpy(buf, testdata[i], len);
		buf[len++] = '\n';

		// 데이터 보내기
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
