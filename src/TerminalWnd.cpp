#include "TerminalWnd.h"

#include <Windowsx.h>

#include "Machine.h"
#include "WindowsPlus.h"
#include "WindowMgr.h"
#include "Utils.h"

TCHAR* pTerminalWndClass = TEXT("RadTerminalWnd");

namespace{
    const HFONT g_hFont = (HFONT) GetStockObject(OEM_FIXED_FONT);
    TEXTMETRIC g_tm = {};
    const zuint16 ScreenMem = 0x8000;
    const zuint16 CursorXLoc = 0xF101;
    const zuint16 CursorYLoc = 0xF102;
    const int ScreenWidth = 80;
    const int ScreenHeight = 25;
    const zuint16 KeyboardBuf = 0xF200;
    const zuint8 KeyboardBufLenMask = 0x0F;
    const zuint16 KeyboardReadLoc = 0xF220;
    const zuint16 KeyboardWriteLoc = 0xF221;
}

LRESULT CALLBACK TerminalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Machine* m = reinterpret_cast<Machine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    MenuWndProc(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_CREATE:
    {
        const CREATESTRUCT* cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        m = reinterpret_cast<Machine*>(cs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(m));
        m->Register(hWnd);

        {
            const HDC hDC = GetDC(hWnd);
            {
                const auto Font = Auto(SelectObject, hDC, (HGDIOBJ) g_hFont);
                GetTextMetrics(hDC, &g_tm);
            }
            ReleaseDC(hWnd, hDC);
        }

        RECT rc = { 0, 0, g_tm.tmAveCharWidth * ScreenWidth, g_tm.tmHeight * ScreenHeight };
        AdjustWindowRect(&rc, GetWindowStyle(hWnd), TRUE);
        SetWindowPos(hWnd, NULL, rc.left, rc.top, Width(rc), Height(rc), SWP_NOMOVE | SWP_NOZORDER);

        return 0;
    }

    case WM_DESTROY:
        m->Unregister(hWnd);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_SETFOCUS:
        CreateCaret(hWnd, NULL, g_tm.tmAveCharWidth, g_tm.tmHeight);
        SetCaretPos(m->memory[CursorXLoc] * g_tm.tmAveCharWidth, m->memory[CursorYLoc] * g_tm.tmHeight);
        ShowCaret(hWnd);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_KILLFOCUS:
        DestroyCaret();
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_ERASEBKGND:
    {
        const HDC hDC = HDC(wParam);
        const auto DCBrushColor = Auto(SetDCBrushColor, hDC, RGB(0, 0, 0));
        const auto Brush = Auto(SelectObject, hDC, GetStockObject(DC_BRUSH));
        RECT rc;
        GetClientRect(hWnd, &rc);
        Rectangle(hDC, rc.left, rc.top, rc.right + 1, rc.bottom + 1);
        return TRUE;
    }

    case WM_PAINT:
    {
        const int gdirescount = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
        const int userrescount = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        {

            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            const auto MapMode = Auto(SetMapMode, hDC, MM_TEXT);
            const auto Font = Auto(SelectObject, hDC, (HGDIOBJ) g_hFont);
            const auto TextColor = Auto(SetTextColor, hDC, RGB(255, 255, 255));
            const auto BkColor = Auto(SetBkColor, hDC, RGB(0, 0, 0));

            RECT rc = rcClient;
            rc.bottom = rc.top + g_tm.tmHeight;
            zuint16 mem = ScreenMem;
            for (int y = 0; y < ScreenHeight; ++y)
            {
                RECT rcIntersect;
                if (IntersectRect(&rcIntersect, &rc, &ps.rcPaint))
                {
                    TCHAR line[ScreenWidth];
                    for (int x = 0; x < ARRAYSIZE(line); ++x)
                        line[x] = m->memory[mem + x];
                    DrawText(hDC, line, ARRAYSIZE(line), &rc, DT_TOP | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
                }

                OffsetRect(&rc, 0, g_tm.tmHeight);
                mem += ScreenWidth;
            }
        }

        EndPaint(hWnd, &ps);
        ASSERT_EQUAL(gdirescount, GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS));
        //ASSERT_EQUAL(userrescount, GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));

        return 0;
    }

    case WM_CHAR:
    {
        const zuint8 charv = zuint8(wParam);
        const zuint8 readoffset = m->memory[KeyboardReadLoc] & KeyboardBufLenMask;
        const zuint8 writeoffset = m->memory[KeyboardWriteLoc] & KeyboardBufLenMask;
        const zuint8 nextwriteoffset = (writeoffset + 1) & KeyboardBufLenMask;
        if (readoffset != nextwriteoffset)
        {
            m->MemWrite(KeyboardBuf + writeoffset, charv, false);
            m->MemWrite(KeyboardWriteLoc, nextwriteoffset, false);
        }
        else
            MessageBeep(MB_ICONERROR);
        return 0;
    }

    case WM_MEM_CHANGED:
    {
        const zuint16 address = (zuint16)wParam;
        if (address >= ScreenMem && address < (ScreenMem + ScreenHeight * ScreenWidth))
        {
            const zuint16 offset = address - ScreenMem;
            const int x = offset % ScreenWidth;
            const int y = offset / ScreenWidth;
            const RECT rc = Rect({ x * g_tm.tmAveCharWidth, y * g_tm.tmHeight }, { g_tm.tmAveCharWidth, g_tm.tmHeight });
            InvalidateRect(hWnd, &rc, TRUE);
        }
        if (address == CursorXLoc || address == CursorYLoc) // TODO && has keyboard focus
            SetCaretPos(m->memory[CursorXLoc] * g_tm.tmAveCharWidth, m->memory[CursorYLoc] * g_tm.tmHeight);
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
