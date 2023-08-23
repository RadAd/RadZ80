#pragma once
#include <Windows.h>

extern TCHAR* pBreakpointWndClass;

LRESULT CALLBACK BreakpointWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
