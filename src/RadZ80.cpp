#include "resource.h"

#include <Windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <fstream>
#include <string>

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
    while (f.read((char*) &t, sizeof(t)))
    {
        switch (t)
        {
        case 1:
        {
            zuint8 len8 = 0;
            f.read((char*) &len8, sizeof(len8));
            zuint16 len = len8;
            if (len < 3)
                len += 254;

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
    }

    return pc;
}

void LoadBIN(LPCTSTR filename, zuint8* mem, zuint16 offset)
{
    std::ifstream f(filename, std::ios::binary);
    const zuint16 chunk = 256u;
    while (f.read((char*) mem + offset, sizeof(*mem) * chunk))
        offset += chunk;
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

void LoadLBL(LPCTSTR filename, std::map<zuint16, std::tstring>& symbols)
{
    std::tifstream f(filename);
    std::tstring line;
    while (std::getline(f, line))
    {
        if (!line.empty())
        {
            const size_t pos1 = line.find(TEXT(' '));
            const size_t pos2 = line.find(TEXT(' '), pos1 + 1);
            const std::tstring arg1 = line.substr(pos1 + 1, pos2 - pos1 - 1);
            const std::tstring arg2 = line.substr(pos2 + 1);
            symbols[std::stoi(arg1, nullptr, 16)] = arg2;
        }
    }
}

void LoadMAP(LPCTSTR filename, std::map<zuint16, std::tstring>& symbols)
{
    std::tifstream f(filename);
    std::tstring line;
    bool in_symbols = false;
    while (std::getline(f, line))
    {
        if (in_symbols)
        {
            if (!line.empty() && line.front() == ' ')
            {
                const std::tstring arg1 = line.substr(6, 8);
                if (!arg1.empty() && arg1.front() == '-')
                    ;
                else if (!arg1.empty())
                {
                    const std::tstring arg2 = rtrim(line.substr(15, 30));
                    symbols[std::stoi(arg1, nullptr, 16)] = arg2;
                }
            }
            else
                in_symbols = false;
        }
        else if (line == TEXT("      Value  Global                              Global Defined In Module"))
        {
            in_symbols = true;
        }
    }
}

// https://en.wikipedia.org/wiki/Intel_HEX
void LoadIHX(LPCTSTR filename, zuint8* mem)
{
    std::tifstream f(filename);
    std::tstring line;
    while (std::getline(f, line))
    {
        const size_t s = line.find(':');
        if (s != std::tstring::npos)
        {
            size_t offset = s + 1;
            zuint8 sum = 0;
            const int count = std::stoi(line.substr(offset, 2), nullptr, 16);
            sum += count;
            offset += 2;
            const zuint16 address = std::stoi(line.substr(offset, 4), nullptr, 16);
            sum += (address >> 8) & 0xFF;
            sum += address & 0xFF;
            offset += 4;
            const int type = std::stoi(line.substr(offset, 2), nullptr, 16);
            sum += type;
            offset += 2;
            switch (type)
            {
            case 0: // Data
                for (size_t i = 0; i < count; ++i)
                {
                    const zuint8 data = std::stoi(line.substr(offset, 2), nullptr, 16);
                    offset += 2;
                    mem[address + i] = data;
                    sum += data;
                }
                break;

            case 1: // End of file
                if (count != 0)
                    MessageBox(NULL, TEXT("Expect byte count zero at end of file"), TEXT("IHX Error"), MB_OK | MB_ICONERROR);
                break;

            default:
                MessageBox(NULL, TEXT("Unsupported record type"), TEXT("IHX Error"), MB_OK | MB_ICONERROR);
                break;
            }
            const zuint8 checksum = std::stoi(line.substr(offset, 2), nullptr, 16);
            offset += 2;
            const zuint8 check = zuint8(~sum) + 1;
            if (checksum != check)
                MessageBox(NULL, TEXT("Error in checksum"), TEXT("IHX Error"), MB_OK | MB_ICONERROR);
        }
    }
}

std::map<HWND, HACCEL> g_hAccel;

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
        if (ext && _tcsicmp(ext, TEXT(".cmd")) == 0)    // from zmac
        {
            start = LoadCMD(arg, m->memory);
            loaded = true;
        }
        else if (ext && _tcsicmp(ext, TEXT(".lst")) == 0)    // from zmac
        {
            LoadLST(arg, m->symbols);
        }
        else if (ext && _tcsicmp(ext, TEXT(".bin")) == 0)    // from sdcc
        {
            start = 0;
            LoadBIN(arg, m->memory, start);
            loaded = true;
        }
        else if (ext && _tcsicmp(ext, TEXT(".com")) == 0)    // from millfork
        {
            start = 0x100;
            LoadBIN(arg, m->memory, start);
            loaded = true;
        }
        else if (ext && _tcsicmp(ext, TEXT(".lbl")) == 0)    // from millfork
        {
            LoadLBL(arg, m->symbols);
        }
        else if (ext && _tcsicmp(ext, TEXT(".ihx")) == 0)    // from sdcc
        {
            LoadIHX(arg, m->memory);
            start = 0;
            loaded = true;
        }
        else if (ext && _tcsicmp(ext, TEXT(".map")) == 0)    // from sdcc
        {
            LoadMAP(arg, m->symbols);
        }
        else
            MessageBox(NULL, arg, TEXT("Unknown file format"), MB_OK | MB_ICONERROR);
    }
    //m->breakpoint.insert(start + 0x02);
    //m->breakpoint.insert(start + 0x04);
    m->symbols.insert({ start, TEXT("start") });

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

    g_hAccel.insert(std::make_pair(hWndMain, LoadAccelerators(NULL, MAKEINTRESOURCE(IDR_MAIN))));

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) > 0)
    {
        bool found = false;
        for (auto accelpair : g_hAccel)
            if (TranslateAccelerator(accelpair.first, accelpair.second, &msg))
            {
                found = true;
                break;
            }

        if (!found && !IsDialogMessage(g_hWndDlg, &msg))
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
