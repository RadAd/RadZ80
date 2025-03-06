#include "WindowsPlus.h"

#include <crtdbg.h>
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

struct MENUITEMINFOSTR
{
    MENUITEMINFO mii;
    TCHAR str[100];
};

static struct MENUITEMINFOSTR GetMenuItemInfoStr(HMENU hMenu, int i, const UINT fMask)
{
    struct MENUITEMINFOSTR miis;
    ZeroMemory(&miis, sizeof(miis));
    miis.mii.cbSize = sizeof(MENUITEMINFO);
    miis.mii.fMask = fMask;
    if (GetMenuItemInfo(hMenu, i, TRUE, &miis.mii))
    {
        if ((fMask & MIIM_STRING) && miis.mii.cch > 0)
        {
            _ASSERTE(miis.mii.cch < ARRAYSIZE(miis.str));
            ++miis.mii.cch;
            miis.mii.dwTypeData = miis.str;
            GetMenuItemInfo(hMenu, i, TRUE, &miis.mii);
        }
    }
    return miis;
}

static ACCEL* UnpackAccel(HACCEL hAccel)
{
    int count = CopyAcceleratorTable(hAccel, NULL, 0);
    ACCEL* pAccel = malloc((count + 1) * sizeof(ACCEL));
    CopyAcceleratorTable(hAccel, pAccel, count);
    ZeroMemory(&pAccel[count], sizeof(ACCEL));
    return pAccel;
}

static void AccelToString(const ACCEL* pAccel, LPTSTR pStr, size_t len)
{
    pStr[0] = TEXT('\0');
    if (pAccel->fVirt & FALT)
        _tcscat_s(pStr, len, TEXT("Alt+"));
    if (pAccel->fVirt & FCONTROL)
        _tcscat_s(pStr, len, TEXT("Ctrl+"));
    if (pAccel->fVirt & FSHIFT)
        _tcscat_s(pStr, len, TEXT("Shift+"));
    _ASSERTE(pAccel->fVirt & FVIRTKEY);
    {
        const BOOL extended = FALSE;
        const UINT scanCode = MapVirtualKey(pAccel->key, MAPVK_VK_TO_VSC) | (extended ? KF_EXTENDED : 0);
        TCHAR name[100];
        const int result = GetKeyNameText(scanCode << 16, name, ARRAYSIZE(name));
        _tcscat_s(pStr, len, name);
    }
}

static const ACCEL* FindAccel(const ACCEL* pAccel, WORD ID)
{
    for (const ACCEL* pSearchAccel = pAccel; pSearchAccel->cmd != 0; ++pSearchAccel)
    {
        if (pSearchAccel->cmd == ID)
            return pSearchAccel;
    }
    return NULL;
}

void AddAccel(HMENU hMenu, HACCEL hAccel)
{
    ACCEL* pAccel = UnpackAccel(hAccel);
    for (int i = 0; i < GetMenuItemCount(hMenu); ++i)
    {
        struct MENUITEMINFOSTR miis = GetMenuItemInfoStr(hMenu, i, MIIM_ID | MIIM_STRING);
        if (miis.str[0] != TEXT('\0') && _tcschr(miis.str, TEXT('\t')) == NULL)
        {
            const UINT ID = miis.mii.wID;
            const ACCEL* pFoundAccel = FindAccel(pAccel, ID);
            if (pFoundAccel != NULL)
            {
                _tcscat_s(miis.str, ARRAYSIZE(miis.str), TEXT("\t"));
                size_t len = _tcslen(miis.str);
                AccelToString(pFoundAccel, miis.str + len, ARRAYSIZE(miis.str) - len);
                miis.mii.cch = 0;// _tcslen(miis.str);
                miis.mii.dwTypeData = miis.str;
                SetMenuItemInfo(hMenu, i, TRUE, &miis.mii);
            }
        }
    }
    free(pAccel);
}
