#include "RegistersDlg.h"

#include "resource.h"

#include <Windowsx.h>
#include <CommCtrl.h>

#include "Machine.h"
#include "EditPlus.h"
#include "WindowsPlus.h"

#include "WindowMgr.h"

namespace {
    zuint8* GetRegU8(int nIDDlgItem, Machine* m)
    {
        switch (nIDDlgItem)
        {
        case IDC_REG_A: return &Z80_A(m->cpu);
        case IDC_REG_F: return &Z80_F(m->cpu);
        case IDC_REG_B: return &Z80_B(m->cpu);
        case IDC_REG_C: return &Z80_C(m->cpu);
        case IDC_REG_D: return &Z80_D(m->cpu);
        case IDC_REG_E: return &Z80_E(m->cpu);
        case IDC_REG_H: return &Z80_H(m->cpu);
        case IDC_REG_L: return &Z80_L(m->cpu);
        case IDC_REG_A_: return &Z80_A_(m->cpu);
        case IDC_REG_F_: return &Z80_F_(m->cpu);
        case IDC_REG_B_: return &Z80_B_(m->cpu);
        case IDC_REG_C_: return &Z80_C_(m->cpu);
        case IDC_REG_D_: return &Z80_D_(m->cpu);
        case IDC_REG_E_: return &Z80_E_(m->cpu);
        case IDC_REG_H_: return &Z80_H_(m->cpu);
        case IDC_REG_L_: return &Z80_L_(m->cpu);
        default: return nullptr;
        }
    }

    zuint16* GetRegU16(int nIDDlgItem, Machine* m)
    {
        switch (nIDDlgItem)
        {
        case IDC_REG_PC: return &Z80_PC(m->cpu);
        case IDC_REG_SP: return &Z80_SP(m->cpu);
        case IDC_REG_IX: return &Z80_IX(m->cpu);
        case IDC_REG_IY: return &Z80_IY(m->cpu);
        default: return nullptr;
        }
    }

    zuint8 GetFlag(int nIDDlgItem)
    {
        switch (nIDDlgItem)
        {
        case IDC_FLAG_SF: return Z80_SF;
        case IDC_FLAG_ZF: return Z80_ZF;
        case IDC_FLAG_YF: return Z80_YF;
        case IDC_FLAG_HF: return Z80_HF;
        case IDC_FLAG_XF: return Z80_XF;
        case IDC_FLAG_PF: return Z80_PF;
        case IDC_FLAG_NF: return Z80_NF;
        case IDC_FLAG_CF: return Z80_CF;
        case IDC_FLAG_SF_: return Z80_SF;
        case IDC_FLAG_ZF_: return Z80_ZF;
        case IDC_FLAG_YF_: return Z80_YF;
        case IDC_FLAG_HF_: return Z80_HF;
        case IDC_FLAG_XF_: return Z80_XF;
        case IDC_FLAG_PF_: return Z80_PF;
        case IDC_FLAG_NF_: return Z80_NF;
        case IDC_FLAG_CF_: return Z80_CF;
        default: return 0;
        }
    }

    void UpdateRegisters(HWND hWndDlg)
    {
        Machine* m = reinterpret_cast<Machine*>(GetWindowLongPtr(hWndDlg, GWLP_USERDATA));

        TCHAR cmd[100];
        zuint16 pc = Z80_PC(m->cpu);
        Disassemble(m->memory, pc, cmd);
        Replace(cmd, TEXT('\t'), TEXT(' '));
        SetDlgItemText(hWndDlg, IDC_CMD, cmd);

        for (int nIDDlgItem : { IDC_REG_A, IDC_REG_F, IDC_REG_B, IDC_REG_C, IDC_REG_D, IDC_REG_E, IDC_REG_H, IDC_REG_L })
            SetDlgItemHexU8(hWndDlg, nIDDlgItem, *GetRegU8(nIDDlgItem, m));

        const zuint8 regf = Z80_F(m->cpu);
        for (int nIDDlgItem : { IDC_FLAG_SF, IDC_FLAG_ZF, IDC_FLAG_YF, IDC_FLAG_HF, IDC_FLAG_XF, IDC_FLAG_PF, IDC_FLAG_NF, IDC_FLAG_CF })
            Button_SetCheck(GetDlgItem(hWndDlg, nIDDlgItem), regf & GetFlag(nIDDlgItem) ? BST_CHECKED : BST_UNCHECKED);

        for (int nIDDlgItem : { IDC_REG_A_, IDC_REG_F_, IDC_REG_B_, IDC_REG_C_, IDC_REG_D_, IDC_REG_E_, IDC_REG_H_, IDC_REG_L_ })
            SetDlgItemHexU8(hWndDlg, nIDDlgItem, *GetRegU8(nIDDlgItem, m));

        const zuint8 regf_ = Z80_F_(m->cpu);
        for (int nIDDlgItem : { IDC_FLAG_SF_, IDC_FLAG_ZF_, IDC_FLAG_YF_, IDC_FLAG_HF_, IDC_FLAG_XF_, IDC_FLAG_PF_, IDC_FLAG_NF_, IDC_FLAG_CF_ })
            Button_SetCheck(GetDlgItem(hWndDlg, nIDDlgItem), regf_ & GetFlag(nIDDlgItem) ? BST_CHECKED : BST_UNCHECKED);

        for (int nIDDlgItem : { IDC_REG_PC, IDC_REG_SP, IDC_REG_IX, IDC_REG_IY })
            SetDlgItemHexU16(hWndDlg, nIDDlgItem, *GetRegU16(nIDDlgItem, m));
    }
}

INT_PTR CALLBACK DlgRegistersProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    Machine* m = reinterpret_cast<Machine*>(GetWindowLongPtr(hWndDlg, GWLP_USERDATA));

    MenuWndProc(hWndDlg, message, wParam, lParam);

    switch (message)
    {
    case WM_INITDIALOG:
    {
        m = reinterpret_cast<Machine*>(lParam);
        SetWindowLongPtr(hWndDlg, GWLP_USERDATA, LONG_PTR(m));
        m->Register(hWndDlg);

        for (int nIDDlgItem : { IDC_REG_A, IDC_REG_F, IDC_REG_B, IDC_REG_C, IDC_REG_D, IDC_REG_E, IDC_REG_H, IDC_REG_L })
        {
            const HWND hWndEdit = GetDlgItem(hWndDlg, nIDDlgItem);
            Edit_LimitText(hWndEdit, 2);
            SetWindowSubclass(hWndEdit, EditHexChar, 0, 0);
        }
        for (int nIDDlgItem : { IDC_REG_A_, IDC_REG_F_, IDC_REG_B_, IDC_REG_C_, IDC_REG_D_, IDC_REG_E_, IDC_REG_H_, IDC_REG_L_ })
        {
            const HWND hWndEdit = GetDlgItem(hWndDlg, nIDDlgItem);
            Edit_LimitText(hWndEdit, 2);
            SetWindowSubclass(hWndEdit, EditHexChar, 0, 0);
        }
        for (int nIDDlgItem : { IDC_REG_PC, IDC_REG_SP, IDC_REG_IX, IDC_REG_IY })
        {
            const HWND hWndEdit = GetDlgItem(hWndDlg, nIDDlgItem);
            Edit_LimitText(hWndEdit, 4);
            SetWindowSubclass(hWndEdit, EditHexChar, 0, 0);
        }
        SendMessage(hWndDlg, WM_UPDATE_STATE, 0, 0);
        //SetDlgItemText(hWndDlg, IDC_RUN, TEXT("\u23F5"));
        //SetDlgItemText(hWndDlg, IDC_BREAK, TEXT("\u23F9"));
        return TRUE;
    }

    case WM_CTLCOLOREDIT:
    {
        const HDC hDC = HDC(wParam);
        const HWND hEditWnd = HWND(lParam);
        const int id = GetDlgCtrlID(hEditWnd);
        switch (id)
        {
        case IDC_REG_PC:    SetBkColor(hDC, COLOR_PC); break;
        case IDC_REG_SP:    SetBkColor(hDC, COLOR_SP); break;
        case IDC_REG_H: case IDC_REG_L: SetBkColor(hDC, COLOR_HL); break;
        }
        return TRUE;
    }

    case WM_UPDATE_STATE:
        //case WM_CPU_STEP_START:
    case WM_CPU_STEP_STOP:
        switch (m->GetState())
        {
        case State::STOP:
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP), TRUE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP_OVER), TRUE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP_OUT), TRUE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_RUN), TRUE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_BREAK), FALSE);
            UpdateRegisters(hWndDlg);
            break;
        case State::RUN:
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP_OVER), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP_OUT), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_RUN), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_BREAK), TRUE);
            break;
        case State::EXIT:
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP_OVER), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_STEP_OUT), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_RUN), FALSE);
            EnableWindow(GetDlgItem(hWndDlg, IDC_BREAK), FALSE);
            UpdateRegisters(hWndDlg);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        DestroyWindow(hWndDlg);
        return TRUE;

    case WM_DESTROY:
        m->Unregister(hWndDlg);
        return TRUE;

    case WM_COMMAND:
    {
        int nIDDlgItem = LOWORD(wParam);
        switch (nIDDlgItem)
        {
        case IDOK:
        {
            const HWND hWnd = GetFocus();
            nIDDlgItem = GetDlgCtrlID(hWnd);
            switch (nIDDlgItem)
            {
            case IDC_REG_A: case IDC_REG_F: case IDC_REG_B: case IDC_REG_C: case IDC_REG_D: case IDC_REG_E: case IDC_REG_H: case IDC_REG_L:
            case IDC_REG_A_: case IDC_REG_F_: case IDC_REG_B_: case IDC_REG_C_: case IDC_REG_D_: case IDC_REG_E_: case IDC_REG_H_: case IDC_REG_L_:
                *GetRegU8(nIDDlgItem, m) = GetDlgItemHexU8(hWndDlg, nIDDlgItem);
                SetDlgItemHexU8(hWndDlg, nIDDlgItem, *GetRegU8(nIDDlgItem, m));
                break;
            case IDC_REG_PC: case IDC_REG_SP: case IDC_REG_IX: case IDC_REG_IY:
                *GetRegU16(nIDDlgItem, m) = GetDlgItemHexU16(hWndDlg, nIDDlgItem);
                SetDlgItemHexU16(hWndDlg, nIDDlgItem, *GetRegU16(nIDDlgItem, m));
                break;
            }
        }
        break;

        case IDC_REG_A: case IDC_REG_F: case IDC_REG_B: case IDC_REG_C: case IDC_REG_D: case IDC_REG_E: case IDC_REG_H: case IDC_REG_L:
        case IDC_REG_A_: case IDC_REG_F_: case IDC_REG_B_: case IDC_REG_C_: case IDC_REG_D_: case IDC_REG_E_: case IDC_REG_H_: case IDC_REG_L_:
            switch (HIWORD(wParam))
            {
            case EN_KILLFOCUS:
                *GetRegU8(nIDDlgItem, m) = GetDlgItemHexU8(hWndDlg, nIDDlgItem);
                SetDlgItemHexU8(hWndDlg, nIDDlgItem, *GetRegU8(nIDDlgItem, m));
                break;
            }
            break;

        case IDC_REG_PC: case IDC_REG_SP: case IDC_REG_IX: case IDC_REG_IY:
            switch (HIWORD(wParam))
            {
            case EN_KILLFOCUS:
                *GetRegU16(nIDDlgItem, m) = GetDlgItemHexU16(hWndDlg, nIDDlgItem);
                SetDlgItemHexU16(hWndDlg, nIDDlgItem, *GetRegU16(nIDDlgItem, m));
                break;
            }
            break;
        }
        return TRUE;
    }

    default:
        return FALSE;
    }
}
