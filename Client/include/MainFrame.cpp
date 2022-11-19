#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include "FileUtility.h"
#include "Character.h"
#include <random>

#include <windows.h>
#include "Framework.h"
//#include "common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE 50

RECT client;
HWND h_wnd;
PAINTSTRUCT ps;
HDC h_dc;
HDC buf_dc;
HBITMAP buf_bit;
HBITMAP old_bit;
Framework* framework;
std::random_device rd;
std::default_random_engine dre(rd());

HINSTANCE g_h_inst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"windows program";

LRESULT CALLBACK WndProc(HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param);
void PrepareToDoubleBuffering();
void DoubleBuffering();
void CleanUpAfterDoubleBuffering();
void TransmitHDCBufferToRealHDC();
void DrawBuffer(HDC instant_dc, const RECT& rect);
bool MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt);
bool CanGoToPos(const HDC terrain_dc, const POINT pos);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASSEX WndClass;
	g_h_inst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	h_wnd = CreateWindow(lpszClass, lpszWindowName, WS_SYSMENU, 0, 0, 1280, 800, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(h_wnd, nCmdShow);
	UpdateWindow(h_wnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	int retval = 0;
	SOCKET sock;

	const char* testdata[] = {
			"안녕하세요",
			"반가워요",
			"오늘따라 할 이야기는 없네요.",
			"당장 깨어나세요. "
	};
	switch (u_msg)
	{
	case WM_CREATE:
		GetClientRect(h_wnd, &client);
		framework = new Framework;
		SetTimer(h_wnd, 1, 15, 0);
		///// //////////////////////////////////////////
		//윈속 초기화 
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		//소켓 생성 
		 sock = socket(AF_INET, SOCK_STREAM, 0);
		//if (sock == INVALID_SOCKET) err_quit2("socket()");

		//connect()
		struct sockaddr_in serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		//if (retval == SOCKET_ERROR)err_quit2("connect()");

		//데이터 통신에 사용할 변수 
		char buf[BUFSIZE];
		

		//서버와 데이터 통신 
		for (int i = 0; i < 4; i++) {
			//입력 
			memset(buf, '#', sizeof(buf));
			strncpy(buf, testdata[i], strlen(testdata[i]));

			//데이터 보내기 
			retval = send(sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				//err_display2("send()");
				break;
			}
			printf("[TCP클라이언트} %d바이트를 보냈습니다 ,\n", retval);
		}
		closesocket(sock);

		WSACleanup();
		return 0;
	case WM_PAINT:
		PrepareToDoubleBuffering();
		DoubleBuffering();
		CleanUpAfterDoubleBuffering();
		return 0;
	case WM_TIMER:
		framework->Update();
		InvalidateRect(h_wnd, NULL, FALSE);
		return 0;
	case WM_DESTROY:
		delete framework;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(h_wnd, u_msg, w_param, l_param);
}

void PrepareToDoubleBuffering()
{
	h_dc = BeginPaint(h_wnd, &ps);
	buf_dc = CreateCompatibleDC(h_dc);
	buf_bit = CreateCompatibleBitmap(h_dc, client.right, client.bottom);
	old_bit = (HBITMAP)SelectObject(buf_dc, buf_bit);
}

void DoubleBuffering()
{
	framework->Render();

	TransmitHDCBufferToRealHDC();
}

void CleanUpAfterDoubleBuffering()
{
	SelectObject(buf_dc, old_bit);
	DeleteObject(buf_bit);
	DeleteDC(buf_dc);
	EndPaint(h_wnd, &ps);
}

void TransmitHDCBufferToRealHDC()
{
	BitBlt(h_dc, 0, 0, client.right, client.bottom, buf_dc, 0, 0, SRCCOPY);
}

void DrawBuffer(HDC instant_dc, const RECT& rect)
{
	StretchBlt(buf_dc, 0, 0, client.right, client.bottom, instant_dc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SRCCOPY);
}

bool MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt)	// 지형 표시 이미지를 사용해 충돌 확인
{
	if (pt.x < client.left || pt.y > client.right)
		return false;
	if (pt.y < client.top || pt.y > client.bottom)
		return false;

	if (GetPixel(terrain_dc, pt.x, pt.y) == val)
		return true;
	else
		return false;
}

bool CanGoToPos(const HDC terrain_dc, const POINT pos)
{
	if (!MapPixelCollision(terrain_dc, RGB(255, 0, 0), pos))
		return true;
	return false;
}