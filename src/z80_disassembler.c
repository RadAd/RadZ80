// From https://github.com/sarnau/Z80DisAssembler
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>
//typedef char CHAR;
//typedef unsigned char       BYTE;
//typedef unsigned char       UINT8;
//typedef unsigned short      UINT16;

#include <string.h>
#include <stdio.h>

typedef LPCTSTR symbolf(UINT16 adr, const void* data);

void formataddr(TCHAR* s, UINT16 adr, symbolf* pSymbol, const void* data)
{
    LPCTSTR sym = pSymbol(adr, data);
    if (sym)
        _stprintf(s, TEXT("%4.4Xh[%s]"), adr, sym);
    else
        _stprintf(s, TEXT("%4.4Xh"), adr);
}

UINT OpcodeLen(const UINT8* Opcodes, UINT16 adr)
{
    UINT   len = 1;

    switch (Opcodes[adr]) {// Opcode
    case 0x06:          // LD B,n
    case 0x0E:          // LD C,n
    case 0x10:          // DJNZ e
    case 0x16:          // LD D,n
    case 0x18:          // JR e
    case 0x1E:          // LD E,n
    case 0x20:          // JR NZ,e
    case 0x26:          // LD H,n
    case 0x28:          // JR Z,e
    case 0x2E:          // LD L,n
    case 0x30:          // JR NC,e
    case 0x36:          // LD (HL),n
    case 0x38:          // JR C,e
    case 0x3E:          // LD A,n
    case 0xC6:          // ADD A,n
    case 0xCE:          // ADC A,n
    case 0xD3:          // OUT (n),A
    case 0xD6:          // SUB n
    case 0xDB:          // IN A,(n)
    case 0xDE:          // SBC A,n
    case 0xE6:          // AND n
    case 0xEE:          // XOR n
    case 0xF6:          // OR n
    case 0xFE:          // CP n

    case 0xCB:          // Shift-,Rotate-,Bit-Befehle
        len = 2;
        break;
    case 0x01:          // LD BC,nn'
    case 0x11:          // LD DE,nn'
    case 0x21:          // LD HL,nn'
    case 0x22:          // LD (nn'),HL
    case 0x2A:          // LD HL,(nn')
    case 0x31:          // LD SP,(nn')
    case 0x32:          // LD (nn'),A
    case 0x3A:          // LD A,(nn')
    case 0xC2:          // JP NZ,nn'
    case 0xC3:          // JP nn'
    case 0xC4:          // CALL NZ,nn'
    case 0xCA:          // JP Z,nn'
    case 0xCC:          // CALL Z,nn'
    case 0xCD:          // CALL nn'
    case 0xD2:          // JP NC,nn'
    case 0xD4:          // CALL NC,nn'
    case 0xDA:          // JP C,nn'
    case 0xDC:          // CALL C,nn'
    case 0xE2:          // JP PO,nn'
    case 0xE4:          // CALL PO,nn'
    case 0xEA:          // JP PE,nn'
    case 0xEC:          // CALL PE,nn'
    case 0xF2:          // JP P,nn'
    case 0xF4:          // CALL P,nn'
    case 0xFA:          // JP M,nn'
    case 0xFC:          // CALL M,nn'
        len = 3;
        break;
    case 0xDD:  len = 2;
        switch (Opcodes[adr + 1]) {// 2.Teil des Opcodes
        case 0x34:          // INC (IX+d)
        case 0x35:          // DEC (IX+d)
        case 0x46:          // LD B,(IX+d)
        case 0x4E:          // LD C,(IX+d)
        case 0x56:          // LD D,(IX+d)
        case 0x5E:          // LD E,(IX+d)
        case 0x66:          // LD H,(IX+d)
        case 0x6E:          // LD L,(IX+d)
        case 0x70:          // LD (IX+d),B
        case 0x71:          // LD (IX+d),C
        case 0x72:          // LD (IX+d),D
        case 0x73:          // LD (IX+d),E
        case 0x74:          // LD (IX+d),H
        case 0x75:          // LD (IX+d),L
        case 0x77:          // LD (IX+d),A
        case 0x7E:          // LD A,(IX+d)
        case 0x86:          // ADD A,(IX+d)
        case 0x8E:          // ADC A,(IX+d)
        case 0x96:          // SUB A,(IX+d)
        case 0x9E:          // SBC A,(IX+d)
        case 0xA6:          // AND (IX+d)
        case 0xAE:          // XOR (IX+d)
        case 0xB6:          // OR (IX+d)
        case 0xBE:          // CP (IX+d)
            len = 3;
            break;
        case 0x21:          // LD IX,nn'
        case 0x22:          // LD (nn'),IX
        case 0x2A:          // LD IX,(nn')
        case 0x36:          // LD (IX+d),n
        case 0xCB:          // Rotation (IX+d)
            len = 4;
            break;
        }
        break;
    case 0xED:  len = 2;
        switch (Opcodes[adr + 1]) {// 2.Teil des Opcodes
        case 0x43:          // LD (nn'),BC
        case 0x4B:          // LD BC,(nn')
        case 0x53:          // LD (nn'),DE
        case 0x5B:          // LD DE,(nn')
        case 0x73:          // LD (nn'),SP
        case 0x7B:          // LD SP,(nn')
            len = 4;
            break;
        }
        break;
    case 0xFD:  len = 2;
        switch (Opcodes[adr + 1]) {// 2.Teil des Opcodes
        case 0x34:          // INC (IY+d)
        case 0x35:          // DEC (IY+d)
        case 0x46:          // LD B,(IY+d)
        case 0x4E:          // LD C,(IY+d)
        case 0x56:          // LD D,(IY+d)
        case 0x5E:          // LD E,(IY+d)
        case 0x66:          // LD H,(IY+d)
        case 0x6E:          // LD L,(IY+d)
        case 0x70:          // LD (IY+d),B
        case 0x71:          // LD (IY+d),C
        case 0x72:          // LD (IY+d),D
        case 0x73:          // LD (IY+d),E
        case 0x74:          // LD (IY+d),H
        case 0x75:          // LD (IY+d),L
        case 0x77:          // LD (IY+d),A
        case 0x7E:          // LD A,(IY+d)
        case 0x86:          // ADD A,(IY+d)
        case 0x8E:          // ADC A,(IY+d)
        case 0x96:          // SUB A,(IY+d)
        case 0x9E:          // SBC A,(IY+d)
        case 0xA6:          // AND (IY+d)
        case 0xAE:          // XOR (IY+d)
        case 0xB6:          // OR (IY+d)
        case 0xBE:          // CP (IY+d)
            len = 3;
            break;
        case 0x21:          // LD IY,nn'
        case 0x22:          // LD (nn'),IY
        case 0x2A:          // LD IY,(nn')
        case 0x36:          // LD (IY+d),n
        case 0xCB:          // Rotation,Bitop (IY+d)
            len = 4;
            break;
        }
        break;
    }
    return(len);
}

void Disassemble(const UINT8* Opcodes, UINT16 adr, TCHAR* s, symbolf pSymbol, const void* data)
{
    UINT8           a = Opcodes[adr];
    UINT8           d = (a >> 3) & 7;
    UINT8           e = a & 7;
    static TCHAR* reg[8] = { TEXT("B"),TEXT("C"),TEXT("D"),TEXT("E"),TEXT("H"),TEXT("L"),TEXT("(HL)"),TEXT("A") };
    static TCHAR* dreg[4] = { TEXT("BC"),TEXT("DE"),TEXT("HL"),TEXT("SP") };
    static TCHAR* cond[8] = { TEXT("NZ"),TEXT("Z"),TEXT("NC"),TEXT("C"),TEXT("PO"),TEXT("PE"),TEXT("P"),TEXT("M") };
    static TCHAR* arith[8] = { TEXT("ADD\t\tA,"),TEXT("ADC\t\tA,"),TEXT("SUB\t\t"),TEXT("SBC\t\tA,"),TEXT("AND\t\t"),TEXT("XOR\t\t"),TEXT("OR\t\t"),TEXT("CP\t\t") };
    TCHAR            stemp[80];      // temp.String für _stprintf()
    TCHAR            ireg[3];        // temp.Indexregister

    switch (a & 0xC0) {
    case 0x00:
        switch (e) {
        case 0x00:
            switch (d) {
            case 0x00:
                _tcscpy(s, TEXT("NOP"));
                break;
            case 0x01:
                _tcscpy(s, TEXT("EX\t\tAF,AF'"));
                break;
            case 0x02:
                _tcscpy(s, TEXT("DJNZ\t"));
                formataddr(s + _tcslen(s), adr + 2 + (BYTE) Opcodes[adr + 1], pSymbol, data);
                break;
            case 0x03:
                _tcscpy(s, TEXT("JR\t\t"));
                formataddr(s + _tcslen(s), adr + 2 + (BYTE) Opcodes[adr + 1], pSymbol, data);
                break;
            default:
                _tcscpy(s, TEXT("JR\t\t"));
                _tcscat(s, cond[d & 3]);
                _tcscat(s, TEXT(","));
                formataddr(s + _tcslen(s), adr + 2 + (BYTE) Opcodes[adr + 1], pSymbol, data);
                break;
            }
            break;
        case 0x01:
            if (a & 0x08) {
                _tcscpy(s, TEXT("ADD\t\tHL,"));
                _tcscat(s, dreg[d >> 1]);
            }
            else {
                _tcscpy(s, TEXT("LD\t\t"));
                _tcscat(s, dreg[d >> 1]);
                _tcscat(s, TEXT(","));
                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
            }
            break;
        case 0x02:
            switch (d) {
            case 0x00:
                _tcscpy(s, TEXT("LD\t\t(BC),A"));
                break;
            case 0x01:
                _tcscpy(s, TEXT("LD\tA,(BC)"));
                break;
            case 0x02:
                _tcscpy(s, TEXT("LD\t\t(DE),A"));
                break;
            case 0x03:
                _tcscpy(s, TEXT("LD\t\tA,(DE)"));
                break;
            case 0x04:
                _tcscpy(s, TEXT("LD\t\t("));
                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                _tcscat(s, TEXT("),HL"));
                break;
            case 0x05:
                _tcscpy(s, TEXT("LD\t\tHL,("));
                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                _tcscat(s, TEXT(")"));
                break;
            case 0x06:
                _tcscpy(s, TEXT("LD\t\t("));
                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                _tcscat(s, TEXT("),A"));
                break;
            case 0x07:
                _tcscpy(s, TEXT("LD\t\tA,("));
                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                _tcscat(s, TEXT(")"));
                break;
            }
            break;
        case 0x03:
            if (a & 0x08)
                _tcscpy(s, TEXT("DEC\t\t"));
            else
                _tcscpy(s, TEXT("INC\t\t"));
            _tcscat(s, dreg[d >> 1]);
            break;
        case 0x04:
            _tcscpy(s, TEXT("INC\t\t"));
            _tcscat(s, reg[d]);
            break;
        case 0x05:
            _tcscpy(s, TEXT("DEC\t\t"));
            _tcscat(s, reg[d]);
            break;
        case 0x06:              // LD   d,n
            _tcscpy(s, TEXT("LD\t\t"));
            _tcscat(s, reg[d]);
            _tcscat(s, TEXT(","));
            _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
            break;
        case 0x07:
        {
            static TCHAR* str[8] = { TEXT("RLCA"),TEXT("RRCA"),TEXT("RLA"),TEXT("RRA"),TEXT("DAA"),TEXT("CPL"),TEXT("SCF"),TEXT("CCF") };
            _tcscpy(s, str[d]);
        }
        break;
        }
        break;
    case 0x40:                          // LD   d,s
        if (d == e) {
            _tcscpy(s, TEXT("HALT"));
        }
        else {
            _tcscpy(s, TEXT("LD\t\t"));
            _tcscat(s, reg[d]);
            _tcscat(s, TEXT(","));
            _tcscat(s, reg[e]);
        }
        break;
    case 0x80:
        _tcscpy(s, arith[d]);
        _tcscat(s, reg[e]);
        break;
    case 0xC0:
        switch (e) {
        case 0x00:
            _tcscpy(s, TEXT("RET\t\t"));
            _tcscat(s, cond[d]);
            break;
        case 0x01:
            if (d & 1) {
                switch (d >> 1) {
                case 0x00:
                    _tcscpy(s, TEXT("RET"));
                    break;
                case 0x01:
                    _tcscpy(s, TEXT("EXX"));
                    break;
                case 0x02:
                    _tcscpy(s, TEXT("JP\t\t(HL)"));
                    break;
                case 0x03:
                    _tcscpy(s, TEXT("LD\t\tSP,HL"));
                    break;
                }
            }
            else {
                _tcscpy(s, TEXT("POP\t\t"));
                if ((d >> 1) == 3)
                    _tcscat(s, TEXT("AF"));
                else
                    _tcscat(s, dreg[d >> 1]);
            }
            break;
        case 0x02:
            _tcscpy(s, TEXT("JP\t\t"));
            _tcscat(s, cond[d]);
            _tcscat(s, TEXT(","));
            formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
            break;
        case 0x03:
            switch (d) {
            case 0x00:
                _tcscpy(s, TEXT("JP\t\t"));
                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                break;
            case 0x01:                  // 0xCB
                a = Opcodes[++adr];     // Erweiterungsopcode holen
                d = (a >> 3) & 7;
                e = a & 7;
                stemp[1] = 0;           // temp.String = 1 Zeichen
                switch (a & 0xC0) {
                case 0x00:
                {
                    static TCHAR* str[8] = { TEXT("RLC"),TEXT("RRC"),TEXT("RL"),TEXT("RR"),TEXT("SLA"),TEXT("SRA"),TEXT("???"),TEXT("SRL") };
                    _tcscpy(s, str[d]);
                }
                _tcscat(s, TEXT("\t\t"));
                _tcscat(s, reg[e]);
                break;
                case 0x40:
                    _tcscpy(s, TEXT("BIT\t\t"));
                    stemp[0] = d + '0'; _tcscat(s, stemp);
                    _tcscat(s, TEXT(","));
                    _tcscat(s, reg[e]);
                    break;
                case 0x80:
                    _tcscpy(s, TEXT("RES\t\t"));
                    stemp[0] = d + '0'; _tcscat(s, stemp);
                    _tcscat(s, TEXT(","));
                    _tcscat(s, reg[e]);
                    break;
                case 0xC0:
                    _tcscpy(s, TEXT("SET\t\t"));
                    stemp[0] = d + '0'; _tcscat(s, stemp);
                    _tcscat(s, TEXT(","));
                    _tcscat(s, reg[e]);
                    break;
                }
                break;
            case 0x02:
                _tcscpy(s, TEXT("OUT\t\t("));
                _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                _tcscat(s, TEXT("),A"));
                break;
            case 0x03:
                _tcscpy(s, TEXT("IN\t\tA,("));
                _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                _tcscat(s, TEXT(")"));
                break;
            case 0x04:
                _tcscpy(s, TEXT("EX\t\t(SP),HL"));
                break;
            case 0x05:
                _tcscpy(s, TEXT("EX\t\tDE,HL"));
                break;
            case 0x06:
                _tcscpy(s, TEXT("DI"));
                break;
            case 0x07:
                _tcscpy(s, TEXT("EI"));
                break;
            }
            break;
        case 0x04:
            _tcscpy(s, TEXT("CALL\t"));
            _tcscat(s, cond[d]);
            _tcscat(s, TEXT(","));
            formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
            break;
        case 0x05:
            if (d & 1) {
                switch (d >> 1) {
                case 0x00:
                    _tcscpy(s, TEXT("CALL\t"));
                    formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                    break;
                case 0x02:              // 0xED
                    a = Opcodes[++adr]; // Erweiterungsopcode holen
                    d = (a >> 3) & 7;
                    e = a & 7;
                    switch (a & 0xC0) {
                    case 0x40:
                        switch (e) {
                        case 0x00:
                            _tcscpy(s, TEXT("IN\t\t"));
                            _tcscat(s, reg[d]);
                            _tcscat(s, TEXT(",(C)"));
                            break;
                        case 0x01:
                            _tcscpy(s, TEXT("OUT\t\t(C),"));
                            _tcscat(s, reg[d]);
                            break;
                        case 0x02:
                            if (d & 1)
                                _tcscpy(s, TEXT("ADC"));
                            else
                                _tcscpy(s, TEXT("SBC"));
                            _tcscat(s, TEXT("\t\tHL,"));
                            _tcscat(s, dreg[d >> 1]);
                            break;
                        case 0x03:
                            if (d & 1) {
                                _tcscpy(s, TEXT("LD\t\t"));
                                _tcscat(s, dreg[d >> 1]);
                                _tcscat(s, TEXT(",("));
                                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                                _tcscat(s, TEXT(")"));
                            }
                            else {
                                _tcscpy(s, TEXT("LD\t\t("));
                                formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                                _tcscat(s, TEXT("),"));
                                _tcscat(s, dreg[d >> 1]);
                            }
                            break;
                        case 0x04:
                        {
                            static TCHAR* str[8] = { TEXT("NEG"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???") };
                            _tcscpy(s, str[d]);
                        }
                        break;
                        case 0x05:
                        {
                            static TCHAR* str[8] = { TEXT("RETN"),TEXT("RETI"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???") };
                            _tcscpy(s, str[d]);
                        }
                        break;
                        case 0x06:
                            _tcscpy(s, TEXT("IM\t\t"));
                            stemp[0] = d + '0' - 1; stemp[1] = 0;
                            _tcscat(s, stemp);
                            break;
                        case 0x07:
                        {
                            static TCHAR* str[8] = { TEXT("LD\t\tI,A"),TEXT("???"),TEXT("LD\t\tA,I"),TEXT("???"),TEXT("RRD"),TEXT("RLD"),TEXT("???"),TEXT("???") };
                            _tcscpy(s, str[d]);
                        }
                        break;
                        }
                        break;
                    case 0x80:
                    {
                        static TCHAR* str[32] = { TEXT("LDI"),TEXT("CPI"),TEXT("INI"),TEXT("OUTI"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),
                                              TEXT("LDD"),TEXT("CPD"),TEXT("IND"),TEXT("OUTD"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),
                                              TEXT("LDIR"),TEXT("CPIR"),TEXT("INIR"),TEXT("OTIR"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???"),
                                              TEXT("LDDR"),TEXT("CPDR"),TEXT("INDR"),TEXT("OTDR"),TEXT("???"),TEXT("???"),TEXT("???"),TEXT("???") };
                        _tcscpy(s, str[a & 0x1F]);
                    }
                    break;
                    }
                    break;
                default:                // 0x01 (0xDD) = IX, 0x03 (0xFD) = IY
                    _tcscpy(ireg, (a & 0x20) ? TEXT("IY") : TEXT("IX"));
                    a = Opcodes[++adr]; // Erweiterungsopcode holen
                    switch (a) {
                    case 0x09:
                        _tcscpy(s, TEXT("ADD\t\t"));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(",BC"));
                        break;
                    case 0x19:
                        _tcscpy(s, TEXT("ADD\t\t"));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(",DE"));
                        break;
                    case 0x21:
                        _tcscpy(s, TEXT("LD\t\t"));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(","));
                        formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                        break;
                    case 0x22:
                        _tcscpy(s, TEXT("LD\t\t("));
                        formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                        _tcscat(s, TEXT("),"));
                        _tcscat(s, ireg);
                        break;
                    case 0x23:
                        _tcscpy(s, TEXT("INC\t\t"));
                        _tcscat(s, ireg);
                        break;
                    case 0x29:
                        _tcscpy(s, TEXT("ADD\t\t"));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(","));
                        _tcscat(s, ireg);
                        break;
                    case 0x2A:
                        _tcscpy(s, TEXT("LD\t\t"));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(",("));
                        formataddr(s + _tcslen(s), Opcodes[adr + 1] + (Opcodes[adr + 2] << 8), pSymbol, data);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x2B:
                        _tcscpy(s, TEXT("DEC\t\t"));
                        _tcscat(s, ireg);
                        break;
                    case 0x34:
                        _tcscpy(s, TEXT("INC\t\t("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x35:
                        _tcscpy(s, TEXT("DEC\t\t("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x36:
                        _tcscpy(s, TEXT("LD\t\t("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT("),"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 2]); _tcscat(s, stemp);
                        break;
                    case 0x39:
                        _tcscpy(s, TEXT("ADD\t\t"));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(",SP"));
                        break;
                    case 0x46:
                    case 0x4E:
                    case 0x56:
                    case 0x5E:
                    case 0x66:
                    case 0x6E:
                        _tcscpy(s, TEXT("LD\t\t"));
                        _tcscat(s, reg[(a >> 3) & 7]);
                        _tcscat(s, TEXT(",("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x70:
                    case 0x71:
                    case 0x72:
                    case 0x73:
                    case 0x74:
                    case 0x75:
                    case 0x77:
                        _tcscpy(s, TEXT("LD\t\t("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT("),"));
                        _tcscat(s, reg[a & 7]);
                        break;
                    case 0x7E:
                        _tcscpy(s, TEXT("LD\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x86:
                        _tcscpy(s, TEXT("ADD\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x8E:
                        _tcscpy(s, TEXT("ADC\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x96:
                        _tcscpy(s, TEXT("SUB\t\t("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0x9E:
                        _tcscpy(s, TEXT("SBC\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0xA6:
                        _tcscpy(s, TEXT("AND\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0xAE:
                        _tcscpy(s, TEXT("XOR\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0xB6:
                        _tcscpy(s, TEXT("OR\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0xBE:
                        _tcscpy(s, TEXT("CP\t\tA,("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0xE1:
                        _tcscpy(s, TEXT("POP\t\t"));
                        _tcscat(s, ireg);
                        break;
                    case 0xE3:
                        _tcscpy(s, TEXT("EX\t\t(SP),"));
                        _tcscat(s, ireg);
                        break;
                    case 0xE5:
                        _tcscpy(s, TEXT("PUSH\t"));
                        _tcscat(s, ireg);
                        break;
                    case 0xE9:
                        _tcscpy(s, TEXT("JP\t\t("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT(")"));
                        break;
                    case 0xF9:
                        _tcscpy(s, TEXT("LD\t\tSP,"));
                        _tcscat(s, ireg);
                        break;
                    case 0xCB:
                        a = Opcodes[adr + 2]; // weiteren Unteropcode
                        d = (a >> 3) & 7;
                        stemp[1] = 0;
                        switch (a & 0xC0) {
                        case 0x00:
                        {
                            static TCHAR* str[8] = { TEXT("RLC"),TEXT("RRC"),TEXT("RL"),TEXT("RR"),TEXT("SLA"),TEXT("SRA"),TEXT("???"),TEXT("SRL") };
                            _tcscpy(s, str[d]);
                        }
                        _tcscat(s, TEXT("\t\t"));
                        break;
                        case 0x40:
                            _tcscpy(s, TEXT("BIT\t\t"));
                            stemp[0] = d + '0';
                            _tcscat(s, stemp);
                            _tcscat(s, TEXT(","));
                            break;
                        case 0x80:
                            _tcscpy(s, TEXT("RES\t\t"));
                            stemp[0] = d + '0';
                            _tcscat(s, stemp);
                            _tcscat(s, TEXT(","));
                            break;
                        case 0xC0:
                            _tcscpy(s, TEXT("SET\t\t"));
                            stemp[0] = d + '0';
                            _tcscat(s, stemp);
                            _tcscat(s, TEXT(","));
                            break;
                        }
                        _tcscat(s, TEXT("("));
                        _tcscat(s, ireg);
                        _tcscat(s, TEXT("+"));
                        _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
                        _tcscat(s, TEXT(")"));
                        break;
                    }
                    break;
                }
            }
            else {
                _tcscpy(s, TEXT("PUSH\t"));
                if ((d >> 1) == 3)
                    _tcscat(s, TEXT("AF"));
                else
                    _tcscat(s, dreg[d >> 1]);
            }
            break;
        case 0x06:
            _tcscpy(s, arith[d]);
            _stprintf(stemp, TEXT("%2.2Xh"), Opcodes[adr + 1]); _tcscat(s, stemp);
            break;
        case 0x07:
            _tcscpy(s, TEXT("RST\t\t"));
            _stprintf(stemp, TEXT("%2.2Xh"), a & 0x38); _tcscat(s, stemp);
            break;
        }
        break;
    }
}
