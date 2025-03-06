#pragma once
#include <Windows.h>

class Machine;

void RegisterWindows(HINSTANCE hInstance);
HWND ShowWindow(HWND hWnd, UINT nID, Machine* m);
LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
