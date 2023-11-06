#include "resource.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <fstream>
#include <memory>

#include "Machine.h"
#include "WindowsPlus.h"

//#include "MainDlg.h"
#include "TerminalWnd.h"
#include "WindowMgr.h"

HWND g_hWndDlg = NULL;

zuint16 LoadCMD(LPCTSTR filename, zuint8* mem)
{
    zuint16 pc = 0xFFFF;

    std::ifstream f(filename, std::ios::binary);
    zuint8 t = 0;
    f.read((char*) &t, sizeof(t));
    switch (t)
    {
    case 1:
    {
        zuint8 len = 0;
        f.read((char*) &len, sizeof(len));

        zuint16 addr = 0;
        f.read((char*) &addr, sizeof(addr));

        f.read((char*) (mem + addr), sizeof(zuint8) * len);

        if (pc == 0xFFFF)
            pc = addr;

        break;
    }
    default:
        _ASSERT(FALSE);
        break;
    }

    return pc;
}

HACCEL g_hAccel = NULL;

#ifdef _UNICODE
#define tWinMain wWinMain
#define __targv __wargv
#else
#define tWinMain WinMain
#define __targv __argv
#endif
int APIENTRY tWinMain(_In_ const HINSTANCE hInstance, _In_opt_ const HINSTANCE hInstPrev, _In_ const PTSTR cmdline, _In_ const int cmdshow)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const auto m = std::make_unique<Machine>();

    bool loaded = false;
    zuint16 start = 0;
    for (int argi = 1; argi < __argc; ++argi)
    {
        const LPCTSTR arg = __targv[argi];
        {
            start = LoadCMD(arg, m->memory);
            loaded = true;
        }
    }
    //m->breakpoint.insert(start + 0x02);
    //m->breakpoint.insert(start + 0x04);

    z80_power(&m->cpu, TRUE);

    Z80_PC(m->cpu) = start;

    INITCOMMONCONTROLSEX icex;
    ZeroMemory(&icex, sizeof(INITCOMMONCONTROLSEX));
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RegisterWindows(hInstance);

    //const HWND hWndMain = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_REGISTERS), NULL, DlgMainProc, LPARAM(m));
    const HWND hWndMain = CreateWindow(pTerminalWndClass, TEXT("Z80 Terminal"), WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, m.get());
    if (hWndMain == NULL)
    {
        MessageBox(NULL, TEXT("Error creating window"), TEXT("Rad Z80"), MB_OK | MB_ICONERROR);
        return 0;
    }
    ShowWindow(hWndMain, SW_SHOW);

    if (!loaded)
        MessageBox(hWndMain, TEXT("No program loaded"), TEXT("Rad Z80"), MB_OK | MB_ICONERROR);

    g_hAccel = LoadAccelerators(NULL, MAKEINTRESOURCE(IDR_MAIN));

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) > 0)
    {
        if (!IsDialogMessage(g_hWndDlg, &msg) && !TranslateAccelerator(hWndMain, g_hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    if (bRet < 0)
    {
        MessageBox(NULL, TEXT("Error in GetMessage"), TEXT("Rad Z80"), MB_OK | MB_ICONERROR);
    }

    m->Exit();

    return int(msg.wParam);
}
