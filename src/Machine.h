#pragma once

#include <Z80.h>
#include <Windows.h>
#include <thread>
#include <condition_variable>
#include <set>
#include <map>
#include <string>

#ifdef UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#include <algorithm>
inline bool findStringIC(const std::tstring& strHaystack, const std::tstring& strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](TCHAR ch1, TCHAR ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
}

#define WM_UPDATE_STATE (WM_USER + 10)

#define WM_CPU_STEP_START   (WM_USER + 101)
#define WM_CPU_STEP_STOP    (WM_USER + 102)
#define WM_CPU_RUN_START    (WM_USER + 103)
#define WM_CPU_RUN_STOP     (WM_USER + 104)

#define WM_MEM_CHANGED (WM_USER + 111)
#define WM_BREAKPOINT_CHANGED (WM_USER + 112)
#define WM_REG_CHANGED (WM_USER + 113)

#define COLOR_BP RGB(0xFF, 0x00, 0x00)
#define COLOR_PC RGB(0x87, 0xCE, 0xEB)
#define COLOR_SP RGB(0x3C, 0xB0, 0x43)
#define COLOR_HL RGB(0xE9, 0xD7, 0x00)
#define COLOR_IX RGB(0xDA, 0xB6, 0x00)
#define COLOR_IY RGB(0xA9, 0x86, 0x00)

enum class Reg8
{
    None,
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    A_,
    F_,
    B_,
    C_,
    D_,
    E_,
    H_,
    L_,
};

enum class Reg16
{
    None = 0x100,
    PC,
    SP,
    IX,
    IY,
};

// From z80_disassembler.c
typedef LPCTSTR (symbolf)(UINT16 adr, const void* data);
extern "C" void Disassemble(const UINT8* Opcodes, UINT16 adr, TCHAR* s, symbolf* pSymbol, const void* data);
extern "C" UINT OpcodeLen(const UINT8 * Opcodes, UINT16 adr);

LPCTSTR symbol(UINT16 adr, const void* data);

inline bool OpcodeIsCall(const zuint8 opcode)
{
    switch (opcode)
    {
    case 0xC4:          // CALL NZ,nn'
    case 0xCC:          // CALL Z,nn'
    case 0xCD:          // CALL nn'
    case 0xD4:          // CALL NC,nn'
    case 0xDC:          // CALL C,nn'
    case 0xE4:          // CALL PO,nn'
    case 0xEC:          // CALL PE,nn'
    case 0xF4:          // CALL P,nn'
    case 0xFC:          // CALL M,nn'
        return true;
    default:
        return false;
    }
}

enum class State {
    STOP,
    RUN,
    EXIT,
};

class Machine
{
public:
    Machine();

    void Step();
    void Run();
    void Stop();
    void Exit();

    State GetState() const { return s; }
    void Register(HWND hWnd) { hWnds.insert(hWnd); }
    void Unregister(HWND hWnd) { hWnds.erase(hWnd); }

    void MemWrite(zuint16 address, zuint8 value, bool fromemulation);

    void SendRegChanged(Reg8 reg) const { SendAllMessage(WM_REG_CHANGED, 0, (LPARAM) reg); }
    void SendRegChanged(Reg16 reg) const { SendAllMessage(WM_REG_CHANGED, 0, (LPARAM) reg); }

    bool IsBreakPoint(zuint16 address) const { return breakpoint.find(address) != breakpoint.end(); }
    bool IsTempBreakPoint(zuint16 address) const { return tempbreakpoint.find(address) != tempbreakpoint.end(); }
    void SetBreakPoint(zuint16 address, bool set);
    void ToggleBreakPoint(zuint16 address) { SetBreakPoint(address, breakpoint.find(address) == breakpoint.end()); }

    std::map<zuint16, std::tstring>::const_iterator FindSymbol(LPCTSTR s, bool bMatchCase, bool bMatchWholeWord) const
    {
        if (bMatchWholeWord)
        {
            if (bMatchCase)
            {
                for (auto it = symbols.begin(); it != symbols.end(); ++it)
                    if (it->second == s)
                        return it;
            }
            else
            {
                for (auto it = symbols.begin(); it != symbols.end(); ++it)
                    if (lstrcmpi(it->second.c_str(), s) == 0)
                        return it;
            }
        }
        else
        {
            if (bMatchCase)
            {
                for (auto it = symbols.begin(); it != symbols.end(); ++it)
                    if (it->second.find(s) != std::tstring::npos)
                        return it;
            }
            else
            {
                for (auto it = symbols.begin(); it != symbols.end(); ++it)
                    if (findStringIC(it->second.c_str(), s))
                        return it;
            }
        }
        return symbols.end();
    }

    //zusize  cycles;
    zuint8  memory[65536];
    Z80     cpu;
    std::set<zuint16> breakpoint;
    std::set<zuint16> tempbreakpoint;
    bool dobreak;
    zuint16 breakonret;
    std::map<zuint16, std::tstring> symbols;

private:
    void SendAllMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) const;

    State s;
    std::thread runz80;
    std::mutex m;
    std::condition_variable cv;

    std::set<HWND> hWnds;

    friend void RunZ80(Machine* m);
    friend void MachineWrite(void* context, zuint16 address, zuint8 value);
};

inline zuint8* GetRegU8(Reg8 reg, Machine* m)
{
    switch (reg)
    {
    case Reg8::A: return &Z80_A(m->cpu);
    case Reg8::F: return &Z80_F(m->cpu);
    case Reg8::B: return &Z80_B(m->cpu);
    case Reg8::C: return &Z80_C(m->cpu);
    case Reg8::D: return &Z80_D(m->cpu);
    case Reg8::E: return &Z80_E(m->cpu);
    case Reg8::H: return &Z80_H(m->cpu);
    case Reg8::L: return &Z80_L(m->cpu);
    case Reg8::A_: return &Z80_A_(m->cpu);
    case Reg8::F_: return &Z80_F_(m->cpu);
    case Reg8::B_: return &Z80_B_(m->cpu);
    case Reg8::C_: return &Z80_C_(m->cpu);
    case Reg8::D_: return &Z80_D_(m->cpu);
    case Reg8::E_: return &Z80_E_(m->cpu);
    case Reg8::H_: return &Z80_H_(m->cpu);
    case Reg8::L_: return &Z80_L_(m->cpu);
    default: return nullptr;
    }
}

inline zuint16* GetRegU16(Reg16 reg, Machine* m)
{
    switch (reg)
    {
    case Reg16::PC: return &Z80_PC(m->cpu);
    case Reg16::SP: return &Z80_SP(m->cpu);
    case Reg16::IX: return &Z80_IX(m->cpu);
    case Reg16::IY: return &Z80_IY(m->cpu);
    default: return nullptr;
    }
}
