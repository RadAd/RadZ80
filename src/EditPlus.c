#include "EditPlus.h"

#include <ctype.h>
#include <commctrl.h>
#include <tchar.h>

LRESULT CALLBACK EditHexChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_CHAR:
    {
        const TCHAR c = (TCHAR) wParam;
        if (_istprint(c) && !_istxdigit(c))
            return MessageBeep(0), 0;
        else
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}
