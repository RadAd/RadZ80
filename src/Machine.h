#pragma once

#include <Z80.h>
#include <Windows.h>
#include <thread>
#include <condition_variable>
#include <set>

#define WM_UPDATE_STATE (WM_USER + 10)

#define WM_CPU_STEP_START   (WM_USER + 101)
#define WM_CPU_STEP_STOP    (WM_USER + 102)
#define WM_CPU_RUN_START    (WM_USER + 103)
#define WM_CPU_RUN_STOP     (WM_USER + 104)

#define WM_MEM_CHANGED (WM_USER + 111)
#define WM_BREAKPOINT_CHANGED (WM_USER + 112)

#define COLOR_BP RGB(0xFF, 0x00, 0x00)
#define COLOR_PC RGB(0x87, 0xCE, 0xEB)
#define COLOR_SP RGB(0x3C, 0xB0, 0x43)
#define COLOR_HL RGB(0xD6, 0xB8, 0x5A)

// From z80_disassembler.c
extern "C" void Disassemble(const UINT8 * Opcodes, UINT16 adr, TCHAR * s);
extern "C" UINT OpcodeLen(const UINT8 * Opcodes, UINT16 adr);

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

    bool IsBreakPoint(zuint16 address) const { return breakpoint.find(address) != breakpoint.end(); }
    bool IsTempBreakPoint(zuint16 address) const { return tempbreakpoint.find(address) != tempbreakpoint.end(); }
    void ToggleBreakPoint(zuint16 address);

    //zusize  cycles;
    zuint8  memory[65536];
    Z80     cpu;
    std::set<zuint16> breakpoint;
    std::set<zuint16> tempbreakpoint;
    bool dobreak;
    zuint16 breakonret;

private:
    void SendAllMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0);

    State s;
    std::thread runz80;
    std::mutex m;
    std::condition_variable cv;

    std::set<HWND> hWnds;

    friend void RunZ80(Machine* m);
    friend void MachineWrite(void* context, zuint16 address, zuint8 value);
};
