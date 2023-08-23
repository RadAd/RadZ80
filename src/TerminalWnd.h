#pragma once
#include <Windows.h>

extern TCHAR* pTerminalWndClass;

LRESULT CALLBACK TerminalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
