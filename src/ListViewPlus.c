#include "ListViewPlus.h"

#include "WindowsPlus.h"

#include <commctrl.h>
#include <Windowsx.h>

#define EDIT_ID (101)

#define EN_CANCEL           0x0901
#define EN_DONE             0x0902

static LRESULT CALLBACK Edit_InPlaceProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_CHAR:
    {
        TCHAR c = (TCHAR) wParam;
        if (c == TEXT('\x1B'))
            SendMessage(GetParent(hWnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hWnd), EN_CANCEL), (LPARAM) hWnd);
        else if (c == TEXT('\r'))
            SendMessage(GetParent(hWnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hWnd), EN_DONE), (LPARAM) hWnd);
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}

BOOL ListView_EnsureSubItemVisible(HWND hWnd, _In_ int nItem, _In_ int nCol, _In_ BOOL bPartialOK)
{
    if (!ListView_EnsureVisible(hWnd, nItem, bPartialOK))
        return FALSE;

    RECT rect;
    if (!ListView_GetSubItemRect(hWnd, nItem, nCol, LVIR_BOUNDS, &rect))
        return FALSE;

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    if (rect.left < rcClient.left || Width(rect) > Width(rcClient))
        return ListView_Scroll(hWnd, rect.left - rcClient.left, 0);
    else if (rect.right > rcClient.right)
        return ListView_Scroll(hWnd, rect.right - rcClient.right, 0);
    else
        return TRUE;
}

LRESULT CALLBACK ListView_EditSubItemProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        LVHITTESTINFO info;
        ZeroMemory(&info, sizeof(LVHITTESTINFO));
        info.pt.x = GET_X_LPARAM(lParam);
        info.pt.y = GET_Y_LPARAM(lParam);

        if (GetFocus() == hWnd
            && ListView_SubItemHitTest(hWnd, &info) != -1
            && (info.flags & LVHT_ONITEM)
            && (ListView_GetItemState(hWnd, info.iItem, LVIS_FOCUSED) & LVIS_FOCUSED)
            && info.iSubItem > 0)
        {
            if (GetWindowStyle(hWnd) & LVS_EDITLABELS)
                ListView_EditSubLabel(hWnd, info.iItem, info.iSubItem);
            return 0;
        }
        else
        {
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
    }
    break;

    case LVM_EDITLABEL:
    {
        const int nItem = LOWORD(wParam);
        const int nCol = HIWORD(wParam);

        if (!ListView_EnsureSubItemVisible(hWnd, nItem, nCol, TRUE))
            return (LRESULT) NULL;

        RECT rect;
        if (!ListView_GetSubItemRect(hWnd, nItem, nCol, LVIR_BOUNDS, &rect))
            return (LRESULT) NULL;

        TCHAR text[100] = TEXT("");
        ListView_GetItemText(hWnd, nItem, nCol, text, ARRAYSIZE(text));
        text[ARRAYSIZE(text) - 1] = TEXT('\0');

        DWORD dwStyle = WS_BORDER | WS_CHILD | ES_AUTOHSCROLL | ES_UPPERCASE;

        // Get Column alignment
        LV_COLUMN lvcol;
        ZeroMemory(&lvcol, sizeof(LV_COLUMN));
        lvcol.mask = LVCF_FMT;
        if (!ListView_GetColumn(hWnd, nCol, &lvcol))
            return (LRESULT) NULL;
        switch (lvcol.fmt & LVCFMT_JUSTIFYMASK)
        {
        case LVCFMT_LEFT:   dwStyle |= ES_LEFT; break;
        case LVCFMT_RIGHT:  dwStyle |= ES_RIGHT; break;
        default:            dwStyle |= ES_CENTER; break;
        }

#if 0
        SIZE size = {};
        HDC hDC = GetDC(hWnd);
        GetTextExtentPoint(hDC, text, (int) wcslen(text), &size);
        ReleaseDC(hWnd, hDC);

        if (size.cx > Width(rect))
        {
            switch (lvcol.fmt & LVCFMT_JUSTIFYMASK)
            {
            case LVCFMT_LEFT:   rect.right = rect.left + size.cx; break;
            case LVCFMT_RIGHT:  rect.left = rect.right - size.cx; break;
            }
        }
#endif

        HWND hEdit = CreateWindow(
            WC_EDIT,
            text,
            dwStyle,
            rect.left, rect.top, Width(rect), Height(rect),
            hWnd,
            (HMENU) EDIT_ID,
            NULL,
            NULL);

        SetWindowFont(hEdit, GetWindowFont(hWnd), TRUE);
        Edit_SetSel(hEdit, 0, -1);
        SetFocus(hEdit);
        SetWindowLongPtr(hEdit, GWLP_USERDATA, MAKELONG(nItem, nCol));
        SetWindowSubclass(hEdit, Edit_InPlaceProc, 0, 0);

        LV_DISPINFO dispinfo;
        ZeroMemory(&dispinfo, sizeof(LV_DISPINFO));
        dispinfo.hdr.hwndFrom = hWnd;
        dispinfo.hdr.idFrom = GetDlgCtrlID(hWnd);
        dispinfo.hdr.code = LVN_BEGINLABELEDIT;
        dispinfo.item.mask = LVIF_PARAM;
        dispinfo.item.iItem = nItem;
        dispinfo.item.iSubItem = nCol;
        ListView_GetItem(hWnd, &dispinfo.item);

        if (SendMessage(GetParent(hWnd), WM_NOTIFY, dispinfo.hdr.idFrom, (LPARAM) &dispinfo) == FALSE)
            ShowWindow(hEdit, SW_SHOW);
        else
        {
            DestroyWindow(hEdit);
            hEdit = NULL;
        }

        return (LRESULT) hEdit;
    }

    case LVM_GETEDITCONTROL:
    {
        HWND hEdit = GetDlgItem(hWnd, EDIT_ID);
        if (hEdit != NULL)
            return (LRESULT) hEdit;
        else
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case EDIT_ID:
            switch (HIWORD(wParam))
            {
            case EN_CANCEL:
            case EN_DONE:
            case EN_KILLFOCUS:
            {
                HWND hEdit = (HWND) lParam;

                LONG_PTR data = GetWindowLongPtr(hEdit, GWLP_USERDATA);

                TCHAR text[100];
                Edit_GetText(hEdit, text, ARRAYSIZE(text));

                LV_DISPINFO dispinfo;
                ZeroMemory(&dispinfo, sizeof(LV_DISPINFO));
                dispinfo.hdr.hwndFrom = hWnd;
                dispinfo.hdr.idFrom = GetDlgCtrlID(hWnd);
                dispinfo.hdr.code = LVN_ENDLABELEDIT;

                dispinfo.item.mask = LVIF_PARAM;
                dispinfo.item.iItem = LOWORD(data);
                dispinfo.item.iSubItem = HIWORD(data);
                ListView_GetItem(hWnd, &dispinfo.item);

                dispinfo.item.mask |= LVIF_TEXT;
                dispinfo.item.pszText = HIWORD(wParam) != EN_DONE ? NULL : text;

                if (SendMessage(GetParent(hWnd), WM_NOTIFY, dispinfo.hdr.idFrom, (LPARAM) &dispinfo) == TRUE)
                {
                    if (dispinfo.item.pszText != NULL)
                        ListView_SetItemText(hWnd, dispinfo.item.iItem, dispinfo.item.iSubItem, dispinfo.item.pszText);
                }
                DestroyWindow(hEdit);
            }
            break;
            }
            break;
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);

    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}
