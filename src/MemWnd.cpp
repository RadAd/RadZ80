#include "MemWnd.h"

#include <cctype>
#include <cmath>
#include <set>
#include <Windowsx.h>
#include <strsafe.h>
#include <tchar.h>

#include "Machine.h"
#include "EditPlus.h"
#include "ListViewPlus.h"
#include "WindowsPlus.h"

TCHAR* pMemWndClass = TEXT("RadMemWnd");

#define LISTVIEW_ID (100)
#define BYTE_COLUMNS (16)

namespace {
    inline zuint16 GetAddress(const LVITEM* pItem)
    {
        return pItem->iItem * BYTE_COLUMNS + pItem->iSubItem - 1;
    }

    void InvalidateAddress(HWND hWndListView, zuint16 address)
    {
        RECT rc = {};
        ListView_GetSubItemRect(hWndListView, address / BYTE_COLUMNS, address % BYTE_COLUMNS + 1, LVIR_BOUNDS, &rc);
        InvalidateRect(hWndListView, &rc, TRUE);
        ListView_GetSubItemRect(hWndListView, address / BYTE_COLUMNS, BYTE_COLUMNS + 1, LVIR_BOUNDS, &rc);
        InvalidateRect(hWndListView, &rc, TRUE);
    }

    struct MemWndData
    {
        Machine* m;
        std::set<zuint16> changed;
    };

    void MemWndCustomDraw(LPNMLVCUSTOMDRAW lplvcd, const HWND hWnd, const int iItem, const int iSubItem, void* pVoid)
    {
        const MemWndData* data = reinterpret_cast<MemWndData*>(pVoid);
        switch (iSubItem)
        {
        case 0:
            lplvcd->clrText = GetSysColor(COLOR_BTNTEXT);
            lplvcd->clrTextBk = GetSysColor(COLOR_BTNFACE);
            break;

        case BYTE_COLUMNS + 1:
            lplvcd->clrText = RGB(0, 0, 128);
            lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW);
            break;

        default:
        {
            zuint16 address = zuint16(lplvcd->nmcd.dwItemSpec * BYTE_COLUMNS + lplvcd->iSubItem - 1);
            const zuint8 v = data->m->memory[address];
            lplvcd->clrText = v == 0 ? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_WINDOWTEXT);
            if (data->m->GetState() != State::RUN)
            {
                if (address == Z80_PC(data->m->cpu))
                    lplvcd->clrTextBk = COLOR_PC;
                else if (address == Z80_SP(data->m->cpu))
                    lplvcd->clrTextBk = COLOR_SP;
                else if (address == Z80_HL(data->m->cpu))
                    lplvcd->clrTextBk = COLOR_HL;
                else if (address == Z80_IX(data->m->cpu))
                    lplvcd->clrTextBk = COLOR_IX;
                else if (address == Z80_IY(data->m->cpu))
                    lplvcd->clrTextBk = COLOR_IY;
                else if (data->changed.find(address) != data->changed.end())
                    lplvcd->clrTextBk = RGB(255, 0, 0);
                else
                    lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW);
            }
            else
                lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW);
            break;
        }
        }
    }
}

LRESULT CALLBACK MemWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MemWndData* data = reinterpret_cast<MemWndData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        const CREATESTRUCT* cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        data = new MemWndData({ reinterpret_cast<Machine*>(cs->lpCreateParams) });
        SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(data));
        data->m->Register(hWnd);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        HWND hWndListView = ListView_Create(hWnd, rcClient, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS | LVS_OWNERDATA, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT, LISTVIEW_ID);
        SetFocus(hWndListView);

        SetWindowSubclass(hWndListView, ListView_EditSubItemProc, 0, 0);

        HFONT hFont = GetWindowFont(hWndListView);
        LOGFONT lf = {};
        GetObject(hFont, sizeof(lf), &lf);
        lf.lfPitchAndFamily = MONO_FONT | FF_DONTCARE;
        StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), TEXT("Consolas"));
        hFont = CreateFontIndirect(&lf);
        SetWindowFont(hWndListView, hFont, TRUE);
        //DeleteObject(hFont);

        ListView_AddColumn(hWndListView, TEXT("Addr"), LVCFMT_CENTER, 0);

        for (int i = 0; i < BYTE_COLUMNS; ++i)
        {
            TCHAR heading[100];
            StringCchPrintf(heading, ARRAYSIZE(heading), TEXT("%02X"), i);
            ListView_AddColumn(hWndListView, heading, LVCFMT_CENTER, 0);
        }

        ListView_AddColumn(hWndListView, TEXT("ASCII"), LVCFMT_CENTER, 0);

        ListView_SetItemCount(hWndListView, 65536 / BYTE_COLUMNS);

        int width = GetSystemMetrics(SM_CXVSCROLL); // +4;
        const int ColCount = Header_GetItemCount(ListView_GetHeader(hWndListView));
        for (int i = 0; i < ColCount; ++i)
        {
            ListView_SetColumnWidth(hWndListView, i, LVSCW_AUTOSIZE);
            width += ListView_GetColumnWidth(hWndListView, i);
        }

        rcClient.right = rcClient.left + width;
        AdjustWindowRect(&rcClient, GetWindowStyle(hWnd), FALSE);
        SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, Width(rcClient), Height(rcClient), SWP_NOMOVE | SWP_NOZORDER);

        return 0;
    }

    case WM_DESTROY:
    {
        data->m->Unregister(hWnd);
        delete data;
        data = nullptr;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(data));
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
            case LVN_GETDISPINFO:
            {
                const NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pNmHdr;
                if (plvdi->item.mask & LVIF_TEXT)
                {
                    if (plvdi->item.iSubItem == 0)
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%04X"), plvdi->item.iItem * BYTE_COLUMNS);
                    else if (plvdi->item.iSubItem == (BYTE_COLUMNS + 1))
                    {
                        zuint16 index = plvdi->item.iItem * BYTE_COLUMNS;
                        for (int i = 0; i < BYTE_COLUMNS; ++i)
                        {
                            zuint8 v = data->m->memory[index + i];
                            plvdi->item.pszText[i] = std::isprint(v) ? v : TEXT('.');
                        }
                        plvdi->item.pszText[BYTE_COLUMNS] = TEXT('\0');
                    }
                    else
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%02X"), data->m->memory[GetAddress(&plvdi->item)]);
                }
            }
            break;

            case LVN_BEGINLABELEDIT:
            {
                const NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pNmHdr;

                HWND hEdit = ListView_GetEditControl(plvdi->hdr.hwndFrom);
                Edit_LimitText(hEdit, 2);
                SetWindowSubclass(hEdit, EditHexChar, 0, 0);

                return !(plvdi->item.iSubItem >= 1 && plvdi->item.iSubItem < (BYTE_COLUMNS + 1));
            }

            case LVN_ENDLABELEDIT:
            {
                const NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pNmHdr;
                if (plvdi->item.pszText != NULL)
                    data->m->MemWrite(GetAddress(&plvdi->item), (zuint8)_tcstol(plvdi->item.pszText, nullptr, 16), false);
                return FALSE;
            }

            case LVN_ODFINDITEM:
            {
                const LPNMLVFINDITEM pFindInfo = (LPNMLVFINDITEM)pNmHdr;
                //zuint16 address = zuint16(pFindInfo->iStart * BYTE_COLUMNS);
                if (pFindInfo->lvfi.flags & LVFI_STRING)
                {
                    size_t len = 0;
                    StringCchLength(pFindInfo->lvfi.psz, 5, &len);
                    zuint16 address = zuint16(_tcstol(pFindInfo->lvfi.psz, nullptr, 16));
                    //if (len < 4)
                        address *= zuint16(std::pow(0x10, 4 - len));
                    return address / BYTE_COLUMNS;
                }
                else
                    return -1;
            }

            case NM_CUSTOMDRAW:
            {
                return DoListViewCustomDraw(hWnd, message, wParam, lParam, MemWndCustomDraw, data);
            }
            break;
            }
            break;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_SIZE:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        SetWindowPos(hWndListView, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_MEM_CHANGED:
    {
        const zuint16 address = (zuint16)wParam;
        const bool fromemulation = HIWORD(lParam);
        if (fromemulation)
            data->changed.insert(address);

        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        InvalidateAddress(hWndListView, address);
        return 0;
    }

    case WM_REG_CHANGED:
    {
        if (lParam >= (LPARAM) Reg16::None)
        {
            const Reg16 r = (Reg16) lParam;
            const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
            const zuint addr = *GetRegU16(r, data->m);
            InvalidateAddress(hWndListView, addr);
        }
        else
        {
            switch ((Reg8) lParam)
            {
            case Reg8::H: case Reg8::L:
            {
                const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
                const zuint addr = Z80_HL(data->m->cpu);
                InvalidateAddress(hWndListView, addr);
                break;
            }
            }
        }
        return 0;
    }

    case WM_UPDATE_STATE:
    case WM_CPU_STEP_START:
    case WM_CPU_STEP_STOP:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        for (zuint16 address : { Z80_PC(data->m->cpu), Z80_SP(data->m->cpu), Z80_IX(data->m->cpu), Z80_IY(data->m->cpu), Z80_HL(data->m->cpu) })
            InvalidateAddress(hWndListView, address);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
