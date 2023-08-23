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

namespace {
    inline zuint16 GetAddress(const LVITEM* pItem)
    {
        return pItem->iItem * 16 + pItem->iSubItem - 1;
    }

    void InvalidateAddress(HWND hWndListView, zuint16 address)
    {
        RECT rc = {};
        ListView_GetSubItemRect(hWndListView, address / 16, address % 16 + 1, LVIR_BOUNDS, &rc);
        InvalidateRect(hWndListView, &rc, TRUE);
        ListView_GetSubItemRect(hWndListView, address / 16, 17, LVIR_BOUNDS, &rc);
        InvalidateRect(hWndListView, &rc, TRUE);
    }

    struct MemWndData
    {
        Machine* m;
        std::set<zuint16> changed;
    };
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

        for (int i = 0; i < 16; ++i)
        {
            TCHAR heading[100];
            StringCchPrintf(heading, ARRAYSIZE(heading), TEXT("%02X"), i);
            ListView_AddColumn(hWndListView, heading, LVCFMT_CENTER, 0);
        }

        ListView_AddColumn(hWndListView, TEXT("ASCII"), LVCFMT_CENTER, 0);

        ListView_SetItemCount(hWndListView, 65536 / 16);

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

#if 0 // Doesn't seem to work
    case WM_SETFOCUS:
    {
        const HWND hWndChild = GetDlgItem(hWnd, LISTVIEW_ID);
        SetFocus(hWndChild);
        return LRESULT(hWndChild);
    }
#endif

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
                        StringCchPrintf(plvdi->item.pszText, plvdi->item.cchTextMax, TEXT("%04X"), plvdi->item.iItem * 16);
                    else if (plvdi->item.iSubItem == 17)
                    {
                        zuint16 index = plvdi->item.iItem * 16;
                        for (int i = 0; i < 16; ++i)
                        {
                            zuint8 v = data->m->memory[index + i];
                            plvdi->item.pszText[i] = std::isprint(v) ? v : TEXT('.');
                        }
                        plvdi->item.pszText[16] = TEXT('\0');
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

                return !(plvdi->item.iSubItem >= 1 && plvdi->item.iSubItem < 17);
            }

            case LVN_ENDLABELEDIT:
            {
                const NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pNmHdr;
                if (plvdi->item.pszText != NULL)
                    data->m->MemWrite(GetAddress(&plvdi->item), (zuint8)wcstoul(plvdi->item.pszText, nullptr, 16), false);
                return FALSE;
            }

            case LVN_ODFINDITEM:
            {
                const LPNMLVFINDITEM pFindInfo = (LPNMLVFINDITEM)pNmHdr;
                //zuint16 address = zuint16(pFindInfo->iStart * 16);
                if (pFindInfo->lvfi.flags & LVFI_STRING)
                {
                    size_t len = 0;
                    StringCchLength(pFindInfo->lvfi.psz, 5, &len);
                    zuint16 address = zuint16(_tcstol(pFindInfo->lvfi.psz, nullptr, 16));
                    //if (len < 4)
                        address *= zuint16(std::pow(0x10, 4 - len));
                    return address / 16;
                }
                else
                    return -1;
            }

            case NM_CUSTOMDRAW:
            {
                LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNmHdr;
                switch (lplvcd->nmcd.dwDrawStage)
                {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;

                case CDDS_ITEMPREPAINT:
                    lplvcd->clrText = GetSysColor(COLOR_BTNTEXT);
                    lplvcd->clrTextBk = GetSysColor(COLOR_BTNFACE);
                    //return CDRF_NEWFONT;
                    return CDRF_NOTIFYSUBITEMDRAW;

                case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
                    if (lplvcd->iSubItem == 17)
                    {
                        lplvcd->clrText = RGB(0, 0, 128);
                        lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW);
                    }
                    else if (lplvcd->iSubItem != 0)
                    {
                        zuint16 address = zuint16(lplvcd->nmcd.dwItemSpec * 16 + lplvcd->iSubItem - 1);
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
                            else if (data->changed.find(address) != data->changed.end())
                                lplvcd->clrTextBk = RGB(255, 0, 0);
                            else
                                lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW);
                        }
                        else
                            lplvcd->clrTextBk = GetSysColor(COLOR_WINDOW);
                    }
                    //return CDRF_NEWFONT;
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

    case WM_UPDATE_STATE:
    case WM_CPU_STEP_START:
    case WM_CPU_STEP_STOP:
    {
        const HWND hWndListView = GetDlgItem(hWnd, LISTVIEW_ID);
        for (zuint16 address : { Z80_PC(data->m->cpu), Z80_SP(data->m->cpu), Z80_HL(data->m->cpu) })
            InvalidateAddress(hWndListView, address);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
