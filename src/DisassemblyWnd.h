#pragma once
#include <Windows.h>

extern TCHAR* pDisassemblyWndClass;

LRESULT CALLBACK DisassemblyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
