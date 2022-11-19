#include "ServerThread.h"
#include <tchar.h>
#include "FileUtility.h"
#include "Scene.h"
#include "Character.h"

#include <random>

#include <windows.h>

RECT client;
HWND h_wnd;
PAINTSTRUCT ps;
HDC h_dc;
HDC buf_dc;
HBITMAP buf_bit;
HBITMAP old_bit;
Scene* scene;
SOCKET LISTEN; 
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
	HANDLE hThread;
	switch (u_msg)
	{
	case WM_CREATE:
		//������ ���� 
		LISTEN = Create_Listen();
		GetClientRect(h_wnd, &client);
		scene = new Scene;
		SetTimer(h_wnd, 1, 15, 0);
		return 0;
	case WM_PAINT:
		PrepareToDoubleBuffering();
		DoubleBuffering();
		CleanUpAfterDoubleBuffering();
		return 0;
	case WM_TIMER:
		switch (w_param) {
		case 1: //��� ���� ����
			retval = listen(LISTEN, SOMAXCONN);
			if (retval == SOCKET_ERROR) err_quit("listen()");

			SOCKET client_sock;
			struct sockaddr_in clientaddr;
			int addrlen;

			// accept()
			addrlen = sizeof(clientaddr);
			client_sock = accept(LISTEN, (struct sockaddr*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
				break;
			}

			// ������ Ŭ���̾�Ʈ ���� ���
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
				addr, ntohs(clientaddr.sin_port));

			////������ ����!
			 hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)client_sock, 0, NULL);
			if (hThread == NULL) {
				closesocket(client_sock);
			}
			else {
				CloseHandle(hThread);

			}
			break;
		case 2: // Scene update 
			scene->Update();
			InvalidateRect(h_wnd, NULL, FALSE);
			break;
		default: break;
		}
		return 0;
	case WM_DESTROY:
		delete scene;
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
	//scene->Render();

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

bool MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt)	// ���� ǥ�� �̹����� ����� �浹 Ȯ��
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