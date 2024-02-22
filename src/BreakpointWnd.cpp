#include "BreakpointWnd.h"

#include <Windowsx.h>
#include <strsafe.h>

#include "Machine.h"
//#include "EditPlus.h"
#include "ListViewPlus.h"
#include "ListViewVector.h"
#include "WindowsPlus.h"

TCHAR* pBreakpointWndClass = TEXT("RadBreakpointWnd");

#define LISTVIEW_ID (100)

namespace {
    int FindItem(const HWND hWndListView, const zuint16 address)
    {
        ListViewLParamVector lvv(hWndListView);
        RadVectorAdaptor<ListViewLParamVector> v(lvv);
        const auto it = std::lower_bound(v.begin(), v.end(), address);
        return (it != v.end() && *it == address) ? it.index() : -1;
    }

    void InsertItem(const HWND hWndListView, const Machine* m, const int nItem, const zuint16 address)
    {
        TCHAR str[100];
        StringCchPrintf(str, ARRAYSIZE(str), TEXT("%04X"), address);
        ListView_InsertItemTextParam(hWndListView, nItem, str, address);

        Disassemble(m->memory, address, str, &symbol, m);
        Replace(str, TEXT('\t'), TEXT(' '));
        ListView_SetItemText(hWndListView, nItem, 1, str);
    }
}

LRESULT CALLBACK BreakpointWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Machine* m = reinterpret_cast<Machine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        const CREATESTRUCT* cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        m = reinterpret_cast<Machine*>(cs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(m));
        m->Register(hWnd);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        const HWND hWndListView = ListView_Create(hWnd, rcClient, WS_CHILD | WS_VISIBLE | LVS_REPORT, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT, LISTVIEW_ID);
        SetFocus(hWndListView);

        HFONT hFont = GetWindowFont(hWndListView);
        LOGFONT lf = {};
        GetObject(hFont, sizeof(lf), &lf);
        lf.lfPitchAndFamily = MONO_FONT | FF_DONTCARE;
        StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), TEXT("Consolas"));
        hFont = CreateFontIndirect(&lf);
        SetWindowFont(hWndListView, hFont, TRUE);
        //DeleteObject(hFont);

        ListView_AddColumn(hWndListView, TEXT("Address"), LVCFMT_CENTER, 40);
        ListView_AddColumn(hWndListView, TEXT("Opcode"), LVCFMT_LEFT, 160);

        int width = GetSystemMetrics(SM_CXVSCROLL); // +4;
        const int ColCount = ListView_GetColumnCount(hWndListView);
        for (int i = 0; i < ColCount; ++i)
        {
            //ListView_SetColumnWidth(hWndListView, i, LVSCW_AUTOSIZE);
            width += ListView_GetColumnWidth(hWndListView, i);
        }

        rcClient.right = rcClient.left + width;
        AdjustWindowRect(&rcClient, GetWindowStyle(hWnd), FALSE);
        SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, Width(rcClient), Height(rcClient), SWP_NOMOVE | SWP_NOZORDER);

        int nItem = 0;
        for (zuint16 address : m->breakpoint)
        {
            InsertItem(hWndListView, m, nItem, address);
            ++nItem;
        }

        return 0;
    }

    case WM_DESTROY:
        m->Unregister(hWnd);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_SIZE:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        SetWindowPos(hWndListView, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_BREAKPOINT_CHANGED:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        const zuint addr = zuint(wParam);
        const bool isSet = bool(lParam);
        ListViewLParamVector lvv(hWndListView);
        RadVectorAdaptor<ListViewLParamVector> v(lvv);
        const auto it = std::lower_bound(v.begin(), v.end(), addr);
        if (it != v.end() && *it == addr && !isSet)
            v.erase(it);
        else if ((it == v.end() || *it != addr) && isSet)
            InsertItem(hWndListView, m, it.index(), addr);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
