#pragma once
#include <Windows.h>

extern TCHAR* pSymbolsWndClass;

LRESULT CALLBACK SymbolsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
