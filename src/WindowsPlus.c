#include "WindowsPlus.h"

#include <tchar.h>
#include "resource.h"

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

struct InputBoxParam
{
    LPCTSTR strPrompt;
    LPCTSTR strTitle;
    LPTSTR strText;
    INT cchMaxText;
};

static INT_PTR InputBoxDlg(HWND hWndDlg, UINT nCode, WPARAM wParam, LPARAM lParam)
{
    struct InputBoxParam* pibp = (struct InputBoxParam*) GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

    switch (nCode)
    {
    case WM_INITDIALOG:
    {
        pibp = (struct InputBoxParam*) lParam;
        SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR) pibp);
        SetWindowText(hWndDlg, pibp->strTitle);
        SetDlgItemText(hWndDlg, IDC_PROMPT, pibp->strPrompt);
        return TRUE;
    }

    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            GetDlgItemText(hWndDlg, IDC_EDIT1, pibp->strText, pibp->cchMaxText);
            EndDialog(hWndDlg, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hWndDlg, IDCANCEL);
            return TRUE;
        default:
            return FALSE;
        }
    default:
        return FALSE;
    }
}

BOOL InputBox(HWND hWnd, LPCTSTR strPrompt, LPCTSTR strTitle, LPTSTR strText, INT cchMaxText)
{
    const HINSTANCE hInstance = GetModuleHandle(NULL);
    struct InputBoxParam ibp;
    ibp.strPrompt = strPrompt;
    ibp.strTitle = strTitle;
    ibp.strText = strText;
    ibp.cchMaxText = cchMaxText;
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_INPUTBOX), hWnd, InputBoxDlg, (LPARAM) &ibp) == IDOK;
}
