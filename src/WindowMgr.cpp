#include "WindowMgr.h"

#include "RegistersDlg.h"
#include "MemWnd.h"
#include "BreakpointWnd.h"
#include "DisassemblyWnd.h"
#include "TerminalWnd.h"

#include "Machine.h"
#include "WindowsPlus.h"

#include <vector>

#include "resource.h"

extern HACCEL g_hAccel;

namespace
{
    std::vector<ACCEL> UnpackAccel(HACCEL hAccel)
    {
        std::vector<ACCEL> accels(CopyAcceleratorTable(hAccel, nullptr, 0));
        CopyAcceleratorTable(hAccel, accels.data(), int(accels.size()));
        return accels;
    }

    struct MENUITEMINFOSTR : MENUITEMINFO
    {
        typedef std::wstring string;
        string str;

        MENUITEMINFOSTR& operator=(const MENUITEMINFOSTR& other) = delete;
#if 0
        {
            // TODO Copy all data
            if (dwTypeData != nullptr)
                dwTypeData = const_cast<LPTSTR>(str.data());
        }
#endif
    };

    MENUITEMINFOSTR GetMenuItemInfoStr(HMENU hMenu, int i, const UINT fMask)
    {
        MENUITEMINFOSTR mii = {};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = fMask;
        if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
        {
            if ((fMask & MIIM_STRING) && mii.cch > 0)
            {
                mii.str.resize(mii.cch++);
                mii.dwTypeData = const_cast<LPTSTR>(mii.str.data());
                GetMenuItemInfo(hMenu, i, TRUE, &mii);
            }
        }
        return mii;
    }

    class MenuVector
    {
    public:
        MenuVector(HMENU hMenu, UINT fMask)
            : m_hMenu(hMenu), m_fMask(fMask)
        {

        }

        MENUITEMINFOSTR operator[](int i)
        {
            return GetMenuItemInfoStr(m_hMenu, i, m_fMask);
        }

    private:
        HMENU m_hMenu;
        UINT m_fMask;
    };

    std::wstring AccelToString(const ACCEL accel)
    {
        std::wstring str;
        if (accel.fVirt & FALT)
            str += TEXT("Alt+");
        if (accel.fVirt & FCONTROL)
            str += TEXT("Ctrl+");
        if (accel.fVirt & FSHIFT)
            str += TEXT("Shift+");
        _ASSERTE(accel.fVirt & FVIRTKEY);
        {
            const bool extended = false;
            const UINT scanCode = MapVirtualKey(accel.key, MAPVK_VK_TO_VSC) | (extended ? KF_EXTENDED : 0);
            TCHAR name[100];
            const int result = GetKeyNameText(scanCode << 16, name, ARRAYSIZE(name));
            str += name;
        }
        return str;
    }

    void AddAccel(HMENU hMenu, HACCEL hAccel)
    {
        const std::vector<ACCEL> accels = UnpackAccel(hAccel);
        for (int i = 0; i < GetMenuItemCount(hMenu); ++i)
        {
            MENUITEMINFOSTR mii = GetMenuItemInfoStr(hMenu, i, MIIM_ID | MIIM_STRING);
            if (!mii.str.empty())
            {
                const UINT ID = mii.wID;
                auto it = std::find_if(accels.begin(), accels.end(), [ID](const ACCEL& accel) { return accel.cmd == ID; });
                if (it != accels.end())
                {
                    const size_t tab = mii.str.find(TEXT('\t'));
                    if (tab == MENUITEMINFOSTR::string::npos)
                    {
                        mii.str += TEXT("\t");
                        mii.str += AccelToString(*it);
                        //mii.cch = mii.str.length();
                        mii.dwTypeData = const_cast<LPTSTR>(mii.str.data());
                        SetMenuItemInfo(hMenu, i, TRUE, &mii);
                    }
                }
            }
        }
    }
}

void RegisterWindows(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.hInstance = hInstance;

    wc.lpfnWndProc = MemWndProc;
    wc.lpszClassName = pMemWndClass;
    RegisterClass(&wc);

    wc.lpfnWndProc = BreakpointWndProc;
    wc.lpszClassName = pBreakpointWndClass;
    RegisterClass(&wc);

    wc.lpfnWndProc = DisassemblyWndProc;
    wc.lpszClassName = pDisassemblyWndClass;
    RegisterClass(&wc);

    wc.lpfnWndProc = TerminalWndProc;
    wc.lpszClassName = pTerminalWndClass;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RADZ80));
    RegisterClass(&wc);
    wc.lpszMenuName = nullptr;
}

void ShowWindow(HWND hWndTop, LPCTSTR lpClassName, LPCTSTR lpTitle, SIZE sz, bool resizeable, Machine* m)
{
    HWND hWnd = FindOwnedWindow(hWndTop, lpClassName, NULL);
    if (hWnd)
    {
        ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);
    }
    else
    {
        HINSTANCE hInstance = NULL;
        RECT rc;
        GetWindowRect(hWndTop, &rc);
        HWND hWndMem = CreateWindowEx(WS_EX_TOOLWINDOW, lpClassName, lpTitle, WS_OVERLAPPEDWINDOW & ~(resizeable ? 0 : WS_THICKFRAME), rc.right, rc.top, sz.cx, sz.cy, hWndTop, NULL, hInstance, m);
        if (hWndMem == NULL)
            MessageBox(hWndTop, TEXT("Error creating window"), TEXT("Rad Z80"), MB_OK | MB_ICONERROR);
        else
            ShowWindow(hWndMem, SW_SHOW);
    }
}

void ShowDialog(HWND hWndTop, LPCTSTR lpTitle, int Resource, DLGPROC lpDialogFunc, Machine* m)
{
    HWND hWnd = FindOwnedWindow(hWndTop, TEXT("#32770"), lpTitle);
    if (hWnd)
    {
        ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);
    }
    else
    {
        HINSTANCE hInstance = NULL;
        RECT rc;
        GetWindowRect(hWndTop, &rc);
        HWND hWndMem = CreateDialogParam(hInstance, MAKEINTRESOURCE(Resource), hWndTop, lpDialogFunc, LPARAM(m));
        if (hWndMem == NULL)
            MessageBox(hWndTop, TEXT("Error creating dialog"), TEXT("Rad Z80"), MB_OK | MB_ICONERROR);
        else
            ShowWindow(hWndMem, SW_SHOW);
    }
}

LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Machine* m = reinterpret_cast<Machine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    const HWND hWndTop = GetAncestor(hWnd, GA_ROOTOWNER);

    switch (message)
    {
    case WM_DESTROY:
        if (hWndTop == hWnd)
            PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
    {
        int nIDDlgItem = LOWORD(wParam);
        switch (nIDDlgItem)
        {
        case ID_DEBUG_STEP:
        case IDC_STEP:
            m->Step();
            return TRUE;

        case ID_DEBUG_STEPOVER:
        case IDC_STEP_OVER:
        {
            const zuint8 opcode = m->memory[Z80_PC(m->cpu)];
            if (OpcodeIsCall(opcode))
            {
                const zuint16 addr = Z80_PC(m->cpu) + OpcodeLen(m->memory, Z80_PC(m->cpu));
                m->tempbreakpoint.insert(addr);
                m->Run();
            }
            else
                m->Step();
            return TRUE;
        }

        case ID_DEBUG_STEPOUT:
        case IDC_STEP_OUT:
            m->breakonret = Z80_SP(m->cpu);
            m->Run();
            return TRUE;

        case ID_DEBUG_RUN:
        case IDC_RUN:
            m->Run();
            return TRUE;

        case ID_DEBUG_BREAK:
        case IDC_BREAK:
            m->Stop();
            return TRUE;

        case ID_FILE_EXIT:
            DestroyWindow(hWndTop);
            return TRUE;

        case ID_VIEW_OUTPUT:
        {
            const HWND hWnd = GetConsoleWindow();
            SetForegroundWindow(hWnd);
            return TRUE;
        }

        case ID_VIEW_REGISTERS:
            ShowDialog(hWndTop, TEXT("Z80 Registers"), IDD_REGISTERS, DlgRegistersProc, m);
            return TRUE;

        case ID_VIEW_MEMORY:
            ShowWindow(hWndTop, pMemWndClass, TEXT("Z80 Memory"), SIZE({ CW_USEDEFAULT, CW_USEDEFAULT }), true, m);
            return TRUE;

        case ID_VIEW_BREAKPOINTS:
            ShowWindow(hWndTop, pBreakpointWndClass, TEXT("Z80 Breakpoints"), SIZE({ 100, 300 }), true, m);
            return TRUE;

        case ID_VIEW_DISASSEMBLY:
            ShowWindow(hWndTop, pDisassemblyWndClass, TEXT("Z80 Disassembly"), SIZE({ 100, 300 }), true, m);
            return TRUE;
        }
        return FALSE;
    }

    case WM_INITMENUPOPUP:
    {
        const HMENU hMenu = reinterpret_cast<HMENU>(wParam);
        const int nItem = LOWORD(lParam);
        const BOOL bWindow = HIWORD(lParam);
        if (!bWindow)
        {
            AddAccel(hMenu, g_hAccel);
            switch (nItem)
            {
            case 1:
                _ASSERTE(GetMenuItemID(hMenu, 0) == ID_DEBUG_RUN);
                switch (m->GetState())
                {
                case State::STOP:
                    EnableMenuItem(hMenu, ID_DEBUG_STEP, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_STEPOVER, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_STEPOUT, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_RUN, MF_BYCOMMAND | MF_ENABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_BREAK, MF_BYCOMMAND | MF_DISABLED);
                    break;
                case State::RUN:
                    EnableMenuItem(hMenu, ID_DEBUG_STEP, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_STEPOVER, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_STEPOUT, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_RUN, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_BREAK, MF_BYCOMMAND | MF_ENABLED);
                    break;
                case State::EXIT:
                    EnableMenuItem(hMenu, ID_DEBUG_STEP, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_STEPOVER, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_STEPOUT, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_RUN, MF_BYCOMMAND | MF_DISABLED);
                    EnableMenuItem(hMenu, ID_DEBUG_BREAK, MF_BYCOMMAND | MF_DISABLED);
                    break;
                }

                CheckMenuItem(hMenu, ID_DEBUG_RUN, MF_BYCOMMAND | (FindOwnedWindow(hWndTop, pMemWndClass, NULL) ? MF_CHECKED : MF_UNCHECKED));
                break;
            case 2:
                _ASSERTE(GetMenuItemID(hMenu, 0) == ID_VIEW_REGISTERS);
                CheckMenuItem(hMenu, ID_VIEW_REGISTERS, MF_BYCOMMAND | (FindOwnedWindow(hWndTop, TEXT("#32770"), TEXT("Z80 Registers")) ? MF_CHECKED : MF_UNCHECKED));
                CheckMenuItem(hMenu, ID_VIEW_MEMORY, MF_BYCOMMAND | (FindOwnedWindow(hWndTop, pMemWndClass, NULL) ? MF_CHECKED : MF_UNCHECKED));
                CheckMenuItem(hMenu, ID_VIEW_BREAKPOINTS, MF_BYCOMMAND | (FindOwnedWindow(hWndTop, pBreakpointWndClass, NULL) ? MF_CHECKED : MF_UNCHECKED));
                CheckMenuItem(hMenu, ID_VIEW_DISASSEMBLY, MF_BYCOMMAND | (FindOwnedWindow(hWndTop, pDisassemblyWndClass, NULL) ? MF_CHECKED : MF_UNCHECKED));
                CheckMenuItem(hMenu, ID_VIEW_OUTPUT, MF_BYCOMMAND | (GetConsoleWindow() ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            return FALSE;
        }
        else
            return TRUE;
    }

    default:
        return FALSE;
    }
}
