#include "SymbolsWnd.h"

#include <Windowsx.h>
#include <strsafe.h>
#include <tchar.h>

#include "resource.h"

#include "Machine.h"
//#include "EditPlus.h"
#include "ListViewPlus.h"
#include "ListViewVector.h"
#include "WindowMgr.h"
#include "WindowsPlus.h"

TCHAR* pSymbolsWndClass = TEXT("RadSymbolsWnd");

#define LISTVIEW_ID (100)

namespace {
    void InsertItem(const HWND hWndListView, const Machine* m, const int nItem, const zuint16 address, const std::tstring& name)
    {
        TCHAR str[100];
        StringCchPrintf(str, ARRAYSIZE(str), TEXT("%04X"), address);
        ListView_InsertItemTextParam(hWndListView, nItem, str, address);

        ListView_SetItemText(hWndListView, nItem, 1, (LPWSTR)name.c_str());
    }

    int FindItem(const HWND hWndListView, const zuint16 address)
    {
        ListViewLParamVector lvv(hWndListView);
        RadVectorAdaptor<ListViewLParamVector> v(lvv);
        const auto it = std::lower_bound(v.begin(), v.end(), address);
        return (it != v.end() && *it == address) ? it.index() : -1;
    }

    void InvalidateAddress(HWND hWndListView, zuint16 address, const Machine* m)
    {
        const auto itSymbol = m->symbols.find(address);
        if (itSymbol != m->symbols.end())
        {
            const int iItem = FindItem(hWndListView, address);
            if (iItem >= 0)
            {
                RECT rc = {};
                for (int i = 2; i <= 5; ++i)
                {
                    ListView_GetSubItemRect(hWndListView, iItem, i, LVIR_BOUNDS, &rc);
                    InvalidateRect(hWndListView, &rc, TRUE);
                }
            }
        }
    }
}

LRESULT CALLBACK SymbolsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        ListView_AddColumn(hWndListView, TEXT("Symbol"), LVCFMT_LEFT, 80);
        ListView_AddColumn(hWndListView, TEXT("Byte (hex)"), LVCFMT_RIGHT, 40);
        ListView_AddColumn(hWndListView, TEXT("Byte (dec)"), LVCFMT_RIGHT, 40);
        ListView_AddColumn(hWndListView, TEXT("Word (hex)"), LVCFMT_RIGHT, 60);
        ListView_AddColumn(hWndListView, TEXT("Word (dec)"), LVCFMT_RIGHT, 60);

        int nItem = 0;
        for (const auto& symbol : m->symbols)
        {
            if (symbol.second.compare(0, 2, TEXT("._")) != 0 && symbol.second.compare(0, 1, TEXT("_")) != 0)
            {
                InsertItem(hWndListView, m, nItem, symbol.first, symbol.second);
                ++nItem;
            }
        }

        ListView_SetColumnWidth(hWndListView, 1, LVSCW_AUTOSIZE);

        rcClient.right = rcClient.left + ListView_GetWidth(hWndListView) + GetSystemMetrics(SM_CXVSCROLL);
        AdjustWindowRect(&rcClient, GetWindowStyle(hWnd), FALSE);
        SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, Width(rcClient), Height(rcClient), SWP_NOMOVE | SWP_NOZORDER);

        return 0;
    }

    case WM_DESTROY:
        m->Unregister(hWnd);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_SETFOCUS:
        SetFocus(GetDlgItem(hWnd, LISTVIEW_ID));
        return 0;

    case WM_SIZE:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        SetWindowPos(hWndListView, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_NOTIFY:
    {
        LPNMHDR pNmHdr = LPNMHDR(lParam);
        switch (pNmHdr->idFrom)
        {
        case LISTVIEW_ID:
            switch (pNmHdr->code)
            {
            case LVN_GETDISPINFO:
            {
                const NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pNmHdr;
                if (plvdi->item.mask & LVIF_TEXT)
                {
                    const zuint16 address = (zuint16)ListView_GetItemParam(plvdi->hdr.hwndFrom, plvdi->item.iItem);
                    switch (plvdi->item.iSubItem)
                    {
                    case 2:
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%02X"), m->memory[address]);
                        break;

                    case 3:
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%u"), m->memory[address]);
                        break;

                    case 4:
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%04X"), m->MemReadU16(address));
                        break;

                    case 5:
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%u"), m->MemReadU16(address));
                        break;
                    }
                }
            }
            break;

            case LVN_ITEMACTIVATE:
            {
                const int i = ListView_GetNextItem(pNmHdr->hwndFrom, -1, LVNI_FOCUSED);
                if (i >= 0) // && MessageBox(hWnd, TEXT("Delete breakpoint?"), TEXT("Delete Breakpoint?"), MB_YESNO) == IDYES)
                {
                    const zuint16 address = (zuint16)ListView_GetItemParam(pNmHdr->hwndFrom, i);
                    const HWND hWndTop = GetAncestor(hWnd, GA_ROOTOWNER);
                    const HWND hWndDisassembly = ShowWindow(hWndTop, ID_VIEW_MEMORY, m);
                    SendMessage(hWndDisassembly, WM_GOTO_ADDRESS, 0, address);
                }
                break;
            }
            }
        }
    }

    case WM_MEM_CHANGED:
    {
        const zuint16 address = (zuint16)wParam;
        const bool fromemulation = HIWORD(lParam);
        //if (fromemulation)
            //data->changed.insert(address);

        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        InvalidateAddress(hWndListView, address, m);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
