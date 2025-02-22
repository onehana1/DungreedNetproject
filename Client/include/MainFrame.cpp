#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include "FileUtility.h"
#include "Character.h"
#include <random>

#include <windows.h>
#include "Framework.h"
#include "Thread.h"
//#include "common.h"

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
static char name[20];
static int count = 0;
int g_myid = -1;

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
		count = 0;
		return 0;
	case WM_CHAR:
		switch (framework->GetSceneId()) {
		case 1:
			framework->SetNickname(w_param);
			break;
		}
		//switch (w_param) {
		//case 0x08: //백스페이스 (BackSpace)
		//case 0x09: // 탭 (Tab)
		//case 0x0A: //라인피드(Line Feed) 
		//case 0x00: //캐리지 리턴 (Carriage Return)
		//case 0x1B: //이스케이프(Escape)
		//	MessageBeep((UINT)-1); //비프음
		//	return 0;
		//default:
		//	name[count++] = w_param;
		//	name[count] = '\0';
		//	///InvalidateRect(h_wnd, NULL, TRUE);
		//	return 0;
		//}
	case WM_PAINT:
		PrepareToDoubleBuffering();
		DoubleBuffering();
		CleanUpAfterDoubleBuffering();
		/*h_dc = BeginPaint(h_wnd, &ps);
		GetClientRect(h_wnd, &client);

		TextOut(h_dc, client.right / 2, client.bottom / 2,(LPWSTR)name, strlen(name));*/
		EndPaint(h_wnd, &ps);
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
