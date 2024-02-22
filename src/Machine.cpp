#include "Machine.h"

LPCTSTR symbol(UINT16 adr, const void* data)
{
    const Machine* m = reinterpret_cast<const Machine*>(data);
    auto itSymbol = m->symbols.find(adr);
    if (itSymbol != m->symbols.end())
        return itSymbol->second.c_str();
    else
        return nullptr;
}

void Machine::SendAllMessage(UINT Msg, WPARAM wParam, LPARAM lParam) const
{
    for (HWND hWnd : hWnds)
        SendMessage(hWnd, Msg, wParam, lParam);
}

static void RunZ80(Machine* m)
{
    while (!m->cpu.halt_line && m->cpu.resume != Z80_RESUME_HALT)
    {
        {
            std::unique_lock<std::mutex> lk(m->m);
            m->cv.wait(lk, [m] { return m->s != State::STOP; });
            if (m->s == State::EXIT)
                return;
        }

        m->SendAllMessage(WM_UPDATE_STATE);
        z80_run(&m->cpu, Z80_MAXIMUM_CYCLES);
    }
    m->s = State::EXIT;
    m->SendAllMessage(WM_UPDATE_STATE);
}

void Machine::Step()
{
    SendAllMessage(WM_CPU_STEP_START);
    do
    {
        z80_run(&cpu, 1);
    } while (cpu.resume == Z80_RESUME_XY);
    SendAllMessage(WM_CPU_STEP_STOP);
}

void Machine::Run()
{
    s = State::RUN;
    cv.notify_one();
}

void Machine::Stop()
{
    s = State::STOP;
    tempbreakpoint.clear();
    dobreak = false;
    breakonret = 0xFFFF;
    cpu.cycles = cpu.cycle_limit;
    SendAllMessage(WM_UPDATE_STATE);
}

void Machine::Exit()
{
    s = State::EXIT;
    cv.notify_one();
    runz80.join();
}

void Machine::MemWrite(zuint16 address, zuint8 value, bool fromemulation)
{
    memory[address] = value;
    SendAllMessage(WM_MEM_CHANGED, address, MAKEWORD(value, fromemulation));
}

void Machine::ToggleBreakPoint(zuint16 address)
{
    const auto it = breakpoint.find(address);
    if (it == breakpoint.end())
    {
        breakpoint.insert(address);
        SendAllMessage(WM_BREAKPOINT_CHANGED, address, TRUE);
    }
    else
    {
        breakpoint.erase(it);
        SendAllMessage(WM_BREAKPOINT_CHANGED, address, FALSE);
    }
}

static zuint8 MachineHook(void* context, zuint16 address)
{
    Machine* m = static_cast<Machine*>(context);
    if (m->GetState() == State::EXIT)
        m->cpu.cycles = m->cpu.cycle_limit;
    else
        m->Stop();
    return Z80_HOOK;
}

static zuint8 MachineRead(void* context, zuint16 address)
{
    const Machine* m = static_cast<Machine*>(context);
    return m->memory[address];
}

static zuint8 MachineFetchOpcode(void* context, zuint16 address)
{
    const Machine* m = static_cast<Machine*>(context);
    if (m->cpu.cycles != 0 && (m->GetState() == State::EXIT || m->dobreak || m->IsBreakPoint(address) || m->IsTempBreakPoint(address)))
        return Z80_HOOK;

    return MachineRead(context, address);
}

static void MachineWrite(void* context, zuint16 address, zuint8 value)
{
    Machine* m = static_cast<Machine*>(context);
    m->MemWrite(address, value, true);
}

static void MachineRet(void* context)
{
    Machine* m = static_cast<Machine*>(context);
    if (m->breakonret <= Z80_SP(m->cpu))
        m->dobreak = true;
}

static void MachineOut(void* context, zuint16 address, zuint8 value)
{
    const Machine* m = static_cast<Machine*>(context);
    switch (address & 0xFF)
    {
    case 1:
        if (value == 0x0C)  // Formfeed
            fputs("\u001b[2J\u001b[H", stdout);
        else
            putchar(value);
        break;
    }
}

Machine::Machine()
{
    ZeroMemory(memory, sizeof(Machine::memory));
    ZeroMemory(&cpu, sizeof(Machine::cpu));
    s = State::STOP;
    cpu.context = this;
    cpu.fetch_opcode = MachineFetchOpcode;
    cpu.fetch = MachineRead;
    cpu.read = MachineRead;
    cpu.write = MachineWrite;
    cpu.ret = MachineRet;
    cpu.out = MachineOut;
    cpu.hook = MachineHook;
    breakonret = 0xFFFF;
    runz80 = std::thread(RunZ80, this);
}
