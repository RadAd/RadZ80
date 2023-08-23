#include "EditPlus.h"

#include <ctype.h>
#include <commctrl.h>

LRESULT CALLBACK EditHexChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_CHAR:
    {
        TCHAR c = (TCHAR) wParam;
        if (iswprint(c) && !((c >= TEXT('0') && c <= TEXT('9')) || (c >= TEXT('a') && c <= TEXT('f')) || (c >= TEXT('A') && c <= TEXT('F'))))
            return 0;
        else
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}
