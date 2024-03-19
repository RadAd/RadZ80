#include "DisassemblyWnd.h"

#include <Windowsx.h>
#include <strsafe.h>

#include "Machine.h"
//#include "EditPlus.h"
#include "ListViewPlus.h"
#include "ListViewVector.h"
#include "WindowsPlus.h"
#include "Utils.h"

#include "resource.h"

TCHAR* pDisassemblyWndClass = TEXT("RadDisassemblyWnd");
const HACCEL hAccelDisassembly = LoadAccelerators(NULL, MAKEINTRESOURCE(IDR_DISASSEMBLY));

extern HWND g_hWndDlg;
extern std::map<HWND, HACCEL> g_hAccel;

const UINT WMFindString = RegisterWindowMessage(FINDMSGSTRING);

#define LISTVIEW_ID (100)

namespace {
    int FindItem(const HWND hWndListView, const zuint16 address)
    {
        ListViewLParamVector lvv(hWndListView);
        RadVectorAdaptor<ListViewLParamVector> v(lvv);
        const auto it = std::lower_bound(v.begin(), v.end(), address);
        return (it != v.end() && *it == address) ? it.index() : -1;
    }

    COLORREF GetItemBkColor(const Machine* m, const zuint16 address)
    {
        return m->GetState() != State::RUN && address == Z80_PC(m->cpu) ? COLOR_PC : m->IsBreakPoint(address) ? COLOR_BP : GetSysColor(COLOR_WINDOW);
    }

    void InvalidateItem(const HWND hWndListView, const int nItem)
    {
        RECT rc = {};
        ListView_GetItemRect(hWndListView, nItem, &rc, LVIR_BOUNDS);
        InvalidateRect(hWndListView, &rc, TRUE);
    }

    UINT_PTR FRHookProc(HWND hWnd, UINT nCode, WPARAM wParam, LPARAM lParam)
    {
        switch (nCode)
        {
        case WM_INITDIALOG:
            return TRUE;

        case WM_ACTIVATE:
            if (LOWORD(wParam))
                g_hWndDlg = hWnd;
            else if (g_hWndDlg == hWnd)
                g_hWndDlg = NULL;
            return TRUE;;

        default:
            return 0;
        }
    }

    struct DisassemblyWndData
    {
        Machine* m;
        FINDREPLACE fr;
        TCHAR FindBuffer[100];
    };
}

LRESULT CALLBACK DisassemblyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DisassemblyWndData* data = reinterpret_cast<DisassemblyWndData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    Machine* m = data ? data->m : nullptr;

    if (message == WMFindString)
    {
        LPFINDREPLACE pfr = reinterpret_cast<LPFINDREPLACE>(lParam);
        if (pfr->Flags & FR_FINDNEXT)
        {
            const auto itSymbol = m->FindSymbol(pfr->lpstrFindWhat, pfr->Flags & FR_MATCHCASE, pfr->Flags & FR_WHOLEWORD);
            if (itSymbol != m->symbols.end())
            {
                const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
                const zuint16 addr = itSymbol->first;
                const int nItem = FindItem(hWndListView, addr);
                if (nItem >= 0)
                {
                    ListView_SetItemState(hWndListView, -1, 0, LVIS_SELECTED);
                    ListView_SetItemState(hWndListView, nItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                    ListView_EnsureVisible(hWndListView, nItem, FALSE);
                }
            }
            else
                PlaySound((LPCWSTR) SND_ALIAS_SYSTEMASTERISK, NULL, SND_ALIAS_ID);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    else switch (message)
    {
    case WM_CREATE:
    {
        data = new DisassemblyWndData();

        const auto Cursor = Auto(SetCursor, LoadCursor(NULL, IDC_WAIT));

        const CREATESTRUCT* cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        m = data->m = reinterpret_cast<Machine*>(cs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(data));
        m->Register(hWnd);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        HWND hWndListView = ListView_Create(hWnd, rcClient, WS_CHILD | WS_VISIBLE | LVS_REPORT, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT, LISTVIEW_ID);
        SetFocus(hWndListView);

        HFONT hFont = GetWindowFont(hWndListView);
        LOGFONT lf = {};
        GetObject(hFont, sizeof(lf), &lf);
        lf.lfPitchAndFamily = MONO_FONT | FF_DONTCARE;
        StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), TEXT("Consolas"));
        hFont = CreateFontIndirect(&lf);
        SetWindowFont(hWndListView, hFont, TRUE);

        ListView_AddColumn(hWndListView, TEXT("Address"), LVCFMT_CENTER, 40);
        ListView_AddColumn(hWndListView, TEXT("BP"), LVCFMT_LEFT, 20);
        ListView_AddColumn(hWndListView, TEXT("Symbol"), LVCFMT_LEFT, 80);
        ListView_AddColumn(hWndListView, TEXT("Opcode"), LVCFMT_LEFT, 160);

        {
            zuint16 address = 0x0000;
            int nItem = 0;
            while (address < 0x1000)
            {
                const bool isBreakpoint = m->IsBreakPoint(address);

                TCHAR str[100];
                StringCchPrintf(str, ARRAYSIZE(str), TEXT("%04X"), address);
                ListView_InsertItemTextParam(hWndListView, nItem, str, address);

                if (isBreakpoint)
                    ListView_SetItemText(hWndListView, nItem, 1, TEXT("X"));

                auto itSymbol = m->symbols.find(address);
                if (itSymbol != m->symbols.end())
                    ListView_SetItemText(hWndListView, nItem, 2, (LPWSTR) itSymbol->second.c_str());

                Disassemble(m->memory, address, str, symbol, m);
                Replace(str, TEXT('\t'), TEXT(' '));
                ListView_SetItemText(hWndListView, nItem, 3, str);

                address += OpcodeLen(m->memory, address);
                ++nItem;
            }
        }

        ListView_SetColumnWidth(hWndListView, 2, LVSCW_AUTOSIZE);
        ListView_SetColumnWidth(hWndListView, 3, LVSCW_AUTOSIZE);

        rcClient.right = rcClient.left + ListView_GetWidth(hWndListView) + GetSystemMetrics(SM_CXVSCROLL);
        AdjustWindowRect(&rcClient, GetWindowStyle(hWnd), FALSE);
        SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, Width(rcClient), Height(rcClient), SWP_NOMOVE | SWP_NOZORDER);

        {
            const int nItem = FindItem(hWndListView, Z80_PC(m->cpu));
            if (nItem >= 0)
                ListView_EnsureVisible(hWndListView, nItem, FALSE);
        }

        return 0;
    }

    case WM_DESTROY:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        const HFONT hFont = GetWindowFont(hWndListView);
        DeleteObject(hFont);
        m->Unregister(hWnd);
        delete data;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_SETFOCUS:
        SetFocus(GetDlgItem(hWnd, LISTVIEW_ID));
        return 0;

    case WM_NOTIFY:
    {
        LPNMHDR pNmHdr = LPNMHDR(lParam);
        switch (pNmHdr->idFrom)
        {
        case LISTVIEW_ID:
            switch (pNmHdr->code)
            {
            case LVN_KEYDOWN:
            {
                LPNMLVKEYDOWN lpnkd = (LPNMLVKEYDOWN)pNmHdr;
                if (lpnkd->wVKey == VK_F9)
                {
                    const int i = ListView_GetNextItem(pNmHdr->hwndFrom, -1, LVNI_FOCUSED);
                    const zuint16 address = zuint16(ListView_GetItemParam(pNmHdr->hwndFrom, i));
                    m->ToggleBreakPoint(address);
                }
                break;
            }

            case NM_CUSTOMDRAW:
            {
                LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNmHdr;
                const zuint16 address = (zuint16)ListView_GetItemParam(pNmHdr->hwndFrom, (int)lplvcd->nmcd.dwItemSpec);
                switch (lplvcd->nmcd.dwDrawStage)
                {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;

                case CDDS_ITEMPREPAINT:
                    lplvcd->clrTextBk = GetItemBkColor(m, address);
                    return CDRF_NOTIFYSUBITEMDRAW;

                case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
                    lplvcd->clrTextBk = GetItemBkColor(m, address);
                    return CDRF_DODEFAULT;
                }
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
            }
            break;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_UPDATE_STATE:
    case WM_CPU_STEP_START:
    case WM_CPU_STEP_STOP:
    {
        _ASSERT(m != nullptr);
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        const zuint16 addr = Z80_PC(m->cpu);
        const int nItem = FindItem(hWndListView, addr);
        if (nItem >= 0)
        {
            InvalidateItem(hWndListView, nItem);
            ListView_EnsureVisible(hWndListView, nItem, FALSE);
        }
        return 0;
    }

    case WM_BREAKPOINT_CHANGED:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        const zuint16 addr = zuint16(wParam);
        const bool isSet = bool(lParam);
        const int nItem = FindItem(hWndListView, addr);
        if (nItem >= 0)
        {
            ListView_SetItemText(hWndListView, nItem, 1, isSet ? TEXT("X") : TEXT(""));
            InvalidateItem(hWndListView, nItem);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_REG_CHANGED:
    {
        _ASSERT(m != nullptr);
        if ((Reg16) lParam == Reg16::PC)
        {
            const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
            const zuint16 addr = Z80_PC(m->cpu);
            const int nItem = FindItem(hWndListView, addr);
            if (nItem >= 0)
                InvalidateItem(hWndListView, nItem);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_SIZE:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        SetWindowPos(hWndListView, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_ACTIVATE:
        if (LOWORD(wParam))
            g_hAccel.insert(std::make_pair(hWnd, LoadAccelerators(NULL, MAKEINTRESOURCE(IDR_DISASSEMBLY))));
        else
            g_hAccel.erase(hWnd);
        return TRUE;;

    case WM_COMMAND:
    {
        _ASSERT(data != nullptr);
        const int nIDDlgItem = LOWORD(wParam);
        switch (nIDDlgItem)
        {
        case ID_FIND:
        {
            const HWND hWndFind = FindOwnedWindow(hWnd, TEXT("#32770"), TEXT("Find"));
            if (hWndFind)
                SetActiveWindow(hWndFind);
            else
            {
                data->fr.lStructSize = sizeof(FINDREPLACE);
                data->fr.hwndOwner = hWnd;
                data->fr.Flags = FR_DOWN;
                data->fr.wFindWhatLen = ARRAYSIZE(data->FindBuffer);
                data->fr.lpstrFindWhat = data->FindBuffer;
                data->fr.lpstrFindWhat[0] = TEXT('\0');
                data->fr.Flags |= FR_ENABLEHOOK;
                data->fr.lpfnHook = FRHookProc;

                FindText(&data->fr);
            }
            break;
        }
        }
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
