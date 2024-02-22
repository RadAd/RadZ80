#include "resource.h"

#include <Windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "Machine.h"
#include "WindowsPlus.h"

//#include "MainDlg.h"
#include "TerminalWnd.h"
#include "WindowMgr.h"

#ifdef UNICODE
#define tifstream wifstream
#else
#define tifstream ifstream
#endif

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

inline std::vector<std::tstring> split(const std::tstring& str, TCHAR delim)
{
    std::vector<std::tstring> split;
    std::wstringstream ss(str);
    std::tstring sstr;
    while (std::getline(ss, sstr, delim))
        if (!sstr.empty())
            split.push_back(sstr);
    return split;
}

inline std::tstring ltrim(std::tstring s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](TCHAR ch) {
        return !std::isspace(ch);
        }));
    return s;
}

// trim from end (in place)
inline std::tstring rtrim(std::tstring s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](TCHAR ch) {
        return !std::isspace(ch);
        }).base(), s.end());
    return s;
}

void LoadLST(LPCTSTR filename, std::map<zuint16, std::tstring>& symbols)
{
    std::tifstream f(filename);
    std::tstring line;
    bool in_symbols = false;
    while (std::getline(f, line))
    {
        if (in_symbols)
        {
            const std::tstring arg1 = rtrim(line.substr(0, 15));
            if (!arg1.empty())
            {
                const std::tstring arg2 = line.substr(16, 4);
                symbols[std::stoi(arg2, nullptr, 16)] = arg1;
            }
        }
        else if (line == TEXT("Symbol Table:"))
        {
            in_symbols = true;
        }
    }
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
        const LPCTSTR ext = _tcsrchr(arg, TEXT('.'));
        if (ext && _tcsicmp(ext, TEXT(".cmd")) == 0)
        {
            start = LoadCMD(arg, m->memory);
            loaded = true;
        }
        else if (ext && _tcsicmp(ext, TEXT(".lst")) == 0)
        {
            LoadLST(arg, m->symbols);
        }
        else
            MessageBox(NULL, arg, TEXT("Unknown file format"), MB_OK | MB_ICONERROR);
    }
    //m->breakpoint.insert(start + 0x02);
    //m->breakpoint.insert(start + 0x04);
    m->symbols[start] = TEXT("start");

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
