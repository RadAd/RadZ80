#pragma once
#include <Windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

LRESULT CALLBACK EditHexChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

#ifdef __cplusplus
}
#endif
