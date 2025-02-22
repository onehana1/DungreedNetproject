#pragma once
#ifndef _crosshair
#define _crosshair
#include <windows.h>
#include "Camera.h"
#include "FileUtility.h"
#include "InstantDCset.h"

extern RECT client;
extern HWND h_wnd;

class Camera;

class Crosshair
{
private:
	Image* image;
	int half_size;
public:
	POINT pos;

	Crosshair(int, int);
	Crosshair(const Camera* camera);
	~Crosshair();

	void Init(const Camera* camera);
	void Update(int, int);
	void Update(const Camera* camera);
	void Render(HDC scene_dc, const RECT& bit_rect);

	friend class Player;
	friend class Weapon;
};
#endif
