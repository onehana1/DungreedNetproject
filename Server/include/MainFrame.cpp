
#include "ServerThread.h"
#include <tchar.h>
#include <vector>
#include "FileUtility.h"
#include "Scene.h"
#include "Character.h"

#include <random>

#include <windows.h>

RECT client{ 0, 0, 1264, 761 };
HWND h_wnd;
PAINTSTRUCT ps;
HDC h_dc;
HDC buf_dc;
HBITMAP buf_bit;
HBITMAP old_bit;
Scene* scene = NULL;
std::chrono::duration<double> fps;
SOCKET LISTEN; 
bool game_start = false;
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
int FindEmptyInPlayerList(std::vector<Player*> list);
void DrawBuffer(HDC instant_dc, const RECT& rect);
bool MapPixelCollision(const HDC terrain_dc, const COLORREF& val, const POINT& pt);
bool CanGoToPos(const HDC terrain_dc, const POINT pos);

int main() {
	//������ ���� 
	LISTEN = Create_Listen();
	scene = new Scene;

	HANDLE hThread;
	int retval;
	while (true)
	{
		//printf("packet : %d", sizeof(SC_PLAYER_INPUT_INFO_PACKET));
		//printf("packet : %d", sizeof(PLAYER_INFO) * 3);
		int num = FindEmptyInPlayerList(player_list);

		if(num != -1){
			retval = listen(LISTEN, SOMAXCONN);
			if (retval == SOCKET_ERROR) err_quit("listen()");

			struct sockaddr_in clientaddr;
			int addrlen;

			// accept()
			addrlen = sizeof(clientaddr);

			player_list[num] = new Player;
			player_list[num]->sock = accept(LISTEN, (struct sockaddr*)&clientaddr, &addrlen);
			if (player_list[num]->sock == INVALID_SOCKET) {
				err_display("accept()");
				return 0;
			}

			// ������ Ŭ���̾�Ʈ ���� ���
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
				addr, ntohs(clientaddr.sin_port));

			////������ ����!
			hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)num, 0, NULL);
			player_list[num]->SetState(CONNECT);

			if (hThread == NULL) {
				closesocket(player_list[num]->sock);
				player_list[num]->SetState(UNCONNECT);
			}
			else {
				CloseHandle(hThread);

			}
		}
		else {	// �÷��̾ ���� ��� ���� ���������� ���� ���� ��ȣ�� �Դ��� üũ�Ѵ�
			if (game_start) {
				//scene->SetPlayer(3); // �켱 2 .. ������ Player ���� ���ϰھ �켱 ���� ����. 
				break;
			}
		}
	}
	auto fpsStart = std::chrono::system_clock::now();
	auto fpsEnd = std::chrono::system_clock::now();
	while (true) {
		fps += fpsEnd - fpsStart;
		//cout << fps.count() << endl;
		fpsStart = std::chrono::system_clock::now();

		if (fps.count() > 0.01f) {	// 0.01�ʸ��� update, send
			scene->Update();
			// Ŭ�󿡰� ����/�÷��̾� ���� 
			scene->Send();

			fps = fpsEnd - fpsEnd;
		}
		fpsEnd = std::chrono::system_clock::now();		
	}

	delete scene;
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

int FindEmptyInPlayerList(std::vector<Player*> list)
{
	for (int i = 0; i < list.size(); ++i)
	{
		if (!list[i] || list[i]->GetState() == UNCONNECT) {
			return i;
		}
	}
	return -1;
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
	
	if (GetPixel(terrain_dc, pt.x, pt.y) == val) {
		return true;
	}
	else
		return false;
}


bool MapPixelCollision(const Image* terrain_dc, const COLORREF& val, const POINT& pt)	// ���� ǥ�� �̹����� ����� �浹 Ȯ��
{
	if (pt.x < client.left || pt.y > client.right)
		return false;
	if (pt.y < client.top || pt.y > client.bottom)
		return false;

	if (terrain_dc->GetPixel(pt.x, pt.y) == val) {
		return true;
	}
	else
		return false;
}

bool CanGoToPos(const HDC terrain_dc, const POINT pos)
{
	if (!MapPixelCollision(terrain_dc, RGB(255, 0, 0), pos))
		return true;
	return false;
}