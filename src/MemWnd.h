#pragma once
#include <Windows.h>

extern TCHAR* pMemWndClass;

LRESULT CALLBACK MemWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
