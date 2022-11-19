#pragma once
#pragma once
#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

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

	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int numint = 0;
	int i = 0;
	int readint = 0;
	char buf[BUFSIZE + 1];

	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	//system("cls");


	while (1) {

		// 데이터 받기(고정 길이) - 파일이름
		char file_name[256];
		ZeroMemory(file_name, sizeof(file_name));
		retval = recv(client_sock, file_name, sizeof(file_name), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv(1)");
			break;
		}
		else if (retval == 0)
			break;

		printf("\n 받을 파일 이름 :%s\n", file_name);

		
		CONSOLE_SCREEN_BUFFER_INFO presentCur;
		// 콘솔 출력창의 정보를 담기 위해서 정의한 구조체
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);
		//현재 커서의 위치 정보를 저장하는 함수

		while (1) {
			//retval = WaitForSingleObject(hReadEvent, INFINITE);
			if (retval != WAIT_OBJECT_0) break;
			printf("받음");

			retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);

			//SetEvent(hReadEvent);
		}
		// 받은 데이터 출력
		printf("\n[TCP/%s:%d] %s", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);

	}

	// 소켓 닫기
	//closesocket(client_sock);
	printf("\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	DWORD A = 0;
	return A;
}


