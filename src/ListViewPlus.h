#pragma once
#include <Windows.h>
#include <CommCtrl.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ListView_EditSubLabel(hwndLV, i, c) \
    (HWND)SNDMSG((hwndLV), LVM_EDITLABEL, (WPARAM)(int)(MAKELONG(i, c)), 0L)

inline HWND ListView_Create(HWND hParent, RECT rc, DWORD dwStyle, DWORD dwExStyle, int id)
{
    HWND hWndListView = CreateWindow(
        WC_LISTVIEW,
        TEXT(""),
        dwStyle,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        hParent,
        (HMENU) (INT_PTR) id,
        NULL,
        NULL);
    ListView_SetExtendedListViewStyle(hWndListView, dwExStyle);
    return hWndListView;
}

inline int ListView_GetColumnCount(HWND hWndListView)
{
    return Header_GetItemCount(ListView_GetHeader(hWndListView));
}

inline void ListView_AddColumn(HWND hWndListView, LPCTSTR pTitle, int fmt, int width)
{
    LVCOLUMN lvCol;
    ZeroMemory(&lvCol, sizeof(LVCOLUMN));
    lvCol.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    lvCol.fmt = fmt;
    lvCol.cx = width;
    lvCol.pszText = (LPTSTR) pTitle;

    int iCol = ListView_GetColumnCount(hWndListView);
    ListView_InsertColumn(hWndListView, iCol++, &lvCol);
}

inline void ListView_InsertItemText(HWND hWndListView, int iItem, LPCTSTR pText)
{
    LVITEM item;
    ZeroMemory(&item, sizeof(LVITEM));
    item.iItem = iItem;
    item.mask = LVIF_TEXT;
    item.pszText = (LPTSTR) pText;
    ListView_InsertItem(hWndListView, &item);
}

inline void ListView_InsertItemTextParam(HWND hWndListView, int iItem, LPCTSTR pText, LPARAM lParam)
{
    LVITEM item;
    ZeroMemory(&item, sizeof(LVITEM));
    item.iItem = iItem;
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.pszText = (LPTSTR) pText;
    item.lParam = lParam;
    ListView_InsertItem(hWndListView, &item);
}

inline LPARAM ListView_GetItemParam(HWND hWndListView, int iItem)
{
    LVITEM item;
    ZeroMemory(&item, sizeof(LVITEM));
    item.iItem = iItem;
    item.mask = LVIF_PARAM;
    ListView_GetItem(hWndListView, &item);
    return item.lParam;
}

inline void ListView_SetItemParam(HWND hWndListView, int iItem, LPARAM lParam)
{
    LVITEM item;
    ZeroMemory(&item, sizeof(LVITEM));
    item.iItem = iItem;
    item.mask = LVIF_PARAM;
    item.lParam = lParam;
    ListView_SetItem(hWndListView, &item);
}

BOOL ListView_EnsureSubItemVisible(HWND hWnd, _In_ int nItem, _In_ int nCol, _In_ BOOL bPartialOK);

LRESULT CALLBACK ListView_EditSubItemProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

#ifdef __cplusplus
}

#endif
