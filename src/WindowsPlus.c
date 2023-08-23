#include "WindowsPlus.h"

#include <tchar.h>

struct OwnedWindowData
{
    HWND hOwner;
    LPCTSTR lpClassName;
    LPCTSTR lpWindowName;
    HWND hFound;
};

static BOOL CALLBACK EnumOwnedWindow(_In_ HWND hWnd, _In_ LPARAM lParam)
{
    struct OwnedWindowData* data = (struct OwnedWindowData*) lParam;
    if (GetWindow(hWnd, GW_OWNER) != data->hOwner)
        return TRUE;

    if (data->lpClassName != NULL)
    {
        TCHAR ClassName[1024];
        GetClassName(hWnd, ClassName, ARRAYSIZE(ClassName));
        if (_tcscmp(ClassName, data->lpClassName) != 0)
            return TRUE;
    }

    if (data->lpWindowName != NULL)
    {
        TCHAR WindowName[1024];
        GetWindowText(hWnd, WindowName, ARRAYSIZE(WindowName));
        if (_tcscmp(WindowName, data->lpWindowName) != 0)
            return TRUE;
    }

    data->hFound = hWnd;
    return FALSE;
}

HWND FindOwnedWindow(HWND hOwner, LPCTSTR lpClassName, LPCTSTR lpWindowName)
{
    struct OwnedWindowData data;
    data.hOwner = hOwner;
    data.lpClassName = lpClassName;
    data.lpWindowName = lpWindowName;
    data.hFound = NULL;
    EnumWindows(EnumOwnedWindow, (LPARAM) &data);
    return data.hFound;
}
