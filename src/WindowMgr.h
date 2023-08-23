#pragma once
#include <Windows.h>

class Machine;

void RegisterWindows(HINSTANCE hInstance);
void ShowWindow(HWND hWndDlg, LPCTSTR lpClassName, LPCTSTR lpTitle, SIZE sz, bool resizeable, Machine* m);
LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
