/****************************************************************************

    PROGRAM: e6502.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    Call6502() - execute one instruction
    
    COMMENTS:

    This program is intended to emulate a 6502 processor with it's local 64K
    of data space.  It will also have a neat UI to allow editing of memory,
    and interruption of execution.

****************************************************************************/
#include "emu.h"


#define STACK_START ((unsigned short)0x0100)
#define US(x)	((unsigned short)(x))

/*
 *  Initalize the 6502 machine.
 */
void Init6502(HWND hWnd)
{
    hWnd = hWnd;
    InitRegisters();    
}

/*
 *  Run one instruction.
 *
 *  Does a table lookup and function call from the table.
 */
void Call6502(HWND hWnd)
{
    unsigned register char curr;
   
    if(GetRegister(halt) == HALT)
        return;

    // current value is at the program counter.
    curr = GetRam(GetRegister(pc));

    (instr[curr].fn)(hWnd, curr);
    return;
}


/*
 *  This handles a bad instruction
 *
 *  This opens a MsgBox, and can bring up the CPU Window if it's open.
 */
void BadInstr(HWND hWnd, unsigned char opcode)
{
    char szTemp[80];

    // halt the cpu
    SetRegister(halt, HALT);
    
    // show the user
    wsprintf(szTemp, "Bad opcode 0x%02.02x at 0x%04.04x", opcode, GetRegister(pc));
    MessageBox(hWnd, szTemp, "Invalid Instruction", MB_OK);

#ifdef _DEBUG
    strcat(szTemp, "\r\n");
    
    dPrintf(szTemp);
#endif    
    
    if(IsWindow(hWndCPU))
    {
        SendMessage(hWndCPU, WM_CPUREFRESH, 0, 0L);
        CreateCPUWindow();  /* force a focus and un-minimize */
    }
}

/*
 *  This handles the NO OP code 0xEA
 *
 *  This is also the simplest valid instruction.
 *
 */
void NoOp(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    // remember to bump the PC up over the instruction
    MOVEPC;
    return;
}

/*
 *  Handles all the Load Accumulator commands.
 *
 *  Works by calling the cracker fn and sticking it in A, then setting flags.
 */
void LoadAccum(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;

    r.a = GetVal(opcode);
     
    SIGNBIT(r.a);
    ZEROBIT(r.a);

    MOVEPC;
    return;     
}

/*
 *  Loads the X register
 */
void LoadX(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;

    r.x = GetVal(opcode);
     
    SIGNBIT(r.x);
    ZEROBIT(r.x);

    MOVEPC;
    return;     
}

/*
 *  Loads the Y register
 */
void LoadY(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;

    r.y = GetVal(opcode);
     
    SIGNBIT(r.y);
    ZEROBIT(r.y);

    MOVEPC;
    return;     
}

/*
 *  Clear the carry bit (CLC)
 */
void ClearCarry(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    CLRCARRY;
    MOVEPC;
}

/*
 *  Set the carry bit (SEC)
 */
void SetCarry(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    SETCARRY;
    MOVEPC;
}

/*
 *  Clear the decimal bit (CLD)
 */
void ClearDec(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    r.s = r.s & ~(DECIB);
    MOVEPC;
}

/*
 *  Set the decimal bit (SED)
 */
void SetDec(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    r.s = r.s | DECIB;
    MOVEPC;
}

/*
 *  Clear the overflow bit (CLV)
 */
void ClearOverf(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    r.s = r.s & ~(OVERFB);
    MOVEPC;
}

/*
 *  Set the Interrupt Disable (SEI)
 */
void SetIntr(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    r.s = r.s | INTRB;
    MOVEPC;
}

/*
 *  Clear the Interrupt Disable (CLI)
 */
void ClearIntr(HWND hWnd, unsigned char opcode)
{
    opcode = opcode;
    hWnd = hWnd;
    r.s = r.s & ~(INTRB);
    MOVEPC;
}

/*
 *  Handle the Transfer Register commands, all in one function.
 *  Not too much slower, and lots easier on me.
 */
void TransferReg(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;

    switch(opcode)
    {
        case 0xAA:  // TAX = A -> X
        r.x = r.a;
        SIGNBIT(r.x);
        ZEROBIT(r.x);
        break;

        case 0xA8:  // TAY A -> Y
        r.y = r.a;
        SIGNBIT(r.y);
        ZEROBIT(r.y);
        break;

        case 0xBA:  // TSX sp->x
        r.x = r.sp;
        SIGNBIT(r.x);
        ZEROBIT(r.x);
        break;

        case 0x8A:  // TXA X->A
        r.a = r.x;
        SIGNBIT(r.a);
        ZEROBIT(r.a);
        break;

        case 0x9A:  // TXS x->sp
        r.sp = r.x;
        break;

        case 0x98:  // TYA y->a
        r.a = r.y;
        SIGNBIT(r.a);
        ZEROBIT(r.a);
        break;
    }
    MOVEPC;
}

/*
 *  AndByte
 *
 *  Logically AND's a byte with the accumulator and 
 *  stores the result in the accumulator
 */
void AndByte(HWND hWnd, unsigned char opcode)
{
    unsigned char byte;
    hWnd = hWnd;
    opcode = opcode;

    /* load the correct value to AND */
    byte = GetVal(opcode);

    r.a = r.a & byte;
    SIGNBIT(r.a);
    ZEROBIT(r.a);
    MOVEPC;
}


/*
 *
 */
void AddCarry(HWND hWnd, unsigned char opcode)
{
    unsigned short dest;
    unsigned char val;
    hWnd = hWnd;

    val = GetVal(opcode);
    dest = dest;

    if(r.s & DECIB) // we need to do BCD addition!  eek!
    {
    }
    else            // we're doing binary math
    {
    }
}


/*
 *  preform a one bit left-shift.  The bit shifted off goes to carry,
 *  and a zero is shifted on.
 *
 *  Hey!  Watch the loaing of "val"!  This uses bits that are normally
 *  unused; I special case it here.  Only this, ROR and ROL do this.
 */
void ShiftLeft(HWND hWnd, unsigned char opcode)
{
    unsigned char val;
    hWnd = hWnd;
    opcode = opcode;

    /* Get the right value */
    if(opcode != 0x0A)
        val = GetVal(opcode);
    else
        val = r.a;

    if(val & 0x80)  // save top bit to carry
        SETCARRY;
    else
        CLRCARRY;

    val = val << 1;     // shift

    SIGNBIT(val);
    ZEROBIT(val);

    /* Store the right value */
    if(opcode != 0x0A)
        SetVal(opcode, val);
    else
        r.a = val;

    MOVEPC;
}

/*
 *  Branch
 *
 *  Handle changing the pc by a relative amount.  There are 8 of these that
 *  have seperate opcodes handled by this function.
 */
void Branch(HWND hWnd, unsigned char opcode)
{
    unsigned char type;
    char howmany;

    type = opcode / 0x10;
    howmany = (char)ONEB;   // need sign

    MOVEPC; // do this now; so adding adds the correct number of bytes  

    switch(type)
    {
        case 0x1:   // BPL: branch if positive
        if(!(r.s & SIGNB))
            r.pc += howmany;
        break;

        case 0x3:   // BMI: branch result MInus
        if(r.s & SIGNB)
            r.pc += howmany;
        break;

        case 0x5:   // BVC: branch if overflow clear
        if(!(r.s & OVERFB))
            r.pc += howmany;
        break;

        case 0x7:   // BVS: branch if Overflow set
        if(r.s & OVERFB)
            r.pc += howmany;
        break;

        case 0x9:   // BCC branch if carry clear
        if(!(r.s & CARRYB))
            r.pc += howmany;
        break;

        case 0xB:   // BCS branch if carry set
        if(r.s & CARRYB)
            r.pc += howmany;
        break;

        case 0xD:   // BNE branch not equal (zero clear)
        if(!(r.s & ZEROB))
            r.pc += howmany;
        break;

        case 0xF:   // BEQ branch on equal (zero set)
        if(r.s & ZEROB)
            r.pc += howmany;
        break;

        default:
        dPrintf("undefined branch type %d\r\n", type);
        break;
    }
}


/*
 *  BitByte
 *
 *  so bite back!  Oh, wait.  This is the BIT instruction.  It ands
 *  the accumulator and ram together, sets zero based on that.  It
 *  sets sign and overflow based on the ram byte.  Wierd, huh?
 */
void BitByte(HWND hWnd, unsigned char opcode)
{
    unsigned char byte;
    byte = GetVal(opcode);

    ZEROBIT(byte & r.a);
    SIGNBIT(byte);
    /* set overflow right */
    if(byte & OVERFB) 
        r.s = r.s|OVERFB; 
    else 
        r.s = r.s & (~OVERFB);
    MOVEPC;
}

/*
 *  StoreA
 *
 *  Store the Accumulator at whatever memory location is appropriate due to the
 *  addressing mode.  This is criminally simple due to the opcode cracker.
 */
void StoreA(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    SetVal(opcode, r.a);
    MOVEPC;
}

/*
 *  StoreX
 *
 *  Store the Accumulator at whatever memory location is appropriate due to the
 *  addressing mode.  This is criminally simple due to the opcode cracker.
 */
void StoreX(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    SetVal(opcode, r.x);
    MOVEPC;
}

/*
 *  StoreY
 *
 *  Store the Accumulator at whatever memory location is appropriate due to the
 *  addressing mode.  This is criminally simple due to the opcode cracker.
 */
void StoreY(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    SetVal(opcode, r.y);
    MOVEPC;
}


/*
 *  DecMem
 *
 *  These men work at Digital.  No women, ever.
 *
 *  Oops.  This deincrements a memory location.  This is an oddly easy function.
 */
void DecMem(HWND hWnd, unsigned char opcode)
{
    unsigned char ucMem;
    hWnd = hWnd;
    
    /* load the value, subtract, and save it back */
    ucMem = GetVal(opcode);
    ucMem--;
    SetVal(opcode, ucMem);

    /* save the flags and update the PC */
    SIGNBIT(ucMem);
    ZEROBIT(ucMem);
    MOVEPC;
}


/*
 *  DecX
 *
 *  DEX - sub 1 from the X register, and set flags
 */
void DecX(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    r.x --;
    SIGNBIT(r.x);
    ZEROBIT(r.x);
    MOVEPC;
}


/*
 *  DecY
 *
 *  DEY - sub 1 from the Y register, and set flags
 */
void DecY(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    r.y --;
    SIGNBIT(r.y);
    ZEROBIT(r.y);
    MOVEPC;
}


/*
 *  IncMem
 *
 *  Increment a memory location by one
 */
void IncMem(HWND hWnd, unsigned char opcode)
{
    unsigned char ucMem;
    
    ucMem = GetVal(opcode);
    ucMem++;
    SetVal(opcode, ucMem);
    
    SIGNBIT(ucMem);
    ZEROBIT(ucMem);
    MOVEPC;
}

/*
 *  IncX
 *
 *  Increment the X register
 */
void IncX(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    r.x ++;
    SIGNBIT(r.x);
    ZEROBIT(r.x);
    MOVEPC;
}


/*
 *  IncY
 *
 *  Increment the Y register
 */
void IncY(HWND hWnd, unsigned char opcode)
{
    hWnd = hWnd;
    
    r.y ++;
    SIGNBIT(r.y);
    ZEROBIT(r.y);
    MOVEPC;
}


/*
 *  PushAccum
 *
 *  Pushes the accumulator to the stack, simple simple.
 */
void PushAccum(HWND hWnd, unsigned char opcode)
{
    SetRam(US((STACK_START+r.sp)), r.a); /* store A */
    r.sp--; /* deincrement stack */
    MOVEPC;
}

/*
 *  PopAccum
 *
 *  Pops the stack to the accumulator
 */
void PopAccum(HWND hWnd, unsigned char opcode)
{
    r.sp++; /* increment stack */
    r.a = GetRam(US((STACK_START+r.sp))); /* load A */
    SIGNBIT(r.a);
    ZEROBIT(r.a);
    MOVEPC;
}

/*
 *  PushFlags
 *
 *  Pushes the processor flags to the stack, simple simple.
 */
void PushFlags(HWND hWnd, unsigned char opcode)
{
    SetRam(US((STACK_START+r.sp)), r.s); /* store P */
    r.sp--; /* deincrement stack */
    MOVEPC;
}


/*
 *  PopFlags
 *
 *  Pops the stack to the flags register
 */
void PopFlags(HWND hWnd, unsigned char opcode)
{
    r.sp++; /* increment stack */
    r.s = GetRam(US((STACK_START+r.sp))); /* load A */
    MOVEPC;
}


/*
 *  JumpSub
 *
 *  Boing!  Sandwich!  Boing!  Sandwich!
 *
 *  God, I'm silly.
 *
 *  This is JSR.  It's got only one operating mode, and is actually the first
 *  thing that uses the stack that I've written.
 */
void JumpSub(HWND hWnd, unsigned char opcode)
{
    unsigned short ucOffs;
    unsigned char lsb;
    unsigned char msb;
    
    /* read in and calculate the correct address to jump to... */
    lsb = ONEB;
    msb = TWOB;
    
    ucOffs = lsb + (msb * 0x100);
    
    /* Now, do some strange things and store them on the stack... */

    /* point the pc at the last byte of this... */
    r.pc += 2;
    /* recycle the msb and lsb to the msb and lsb of the program counter */
    msb = (unsigned char)((r.pc & 0xFF00) / 0x100);
    lsb = (unsigned char)r.pc & 0xFF;
    
    /* set the MSB to the stack */
    SetRam(US((STACK_START + r.sp)), msb);
    r.sp--;
    SetRam(US((STACK_START + r.sp)), lsb);
    r.sp--;
    
    MOVEPC; /* this is to get the tick counter */
    r.pc = ucOffs;  /* and JSR to what we are at.. */
}
 

/*
 *  ReturnSub
 *
 *  I said "no pickles", dammit!
 *
 *  This is the oppisite of JSR - it pops the pc off of the stack,
 *  and jumps there.  The neato tla is "RTS" or Return From Subroutine.
 *
 *  Look, I just coded "}"!
 */
void ReturnSub(HWND hWnd, unsigned char opcode)
{
    unsigned short usNewPc;
    unsigned char lsb, msb;
    
    r.sp++;
    lsb = GetRam(US(STACK_START+r.sp));
    r.sp++;
    msb = GetRam(US(STACK_START+r.sp));
    
    usNewPc = lsb + (msb * 0x100);
    usNewPc++;  /* add one as this is really the third byte of JSR */
    
    MOVEPC;     /* get ticks */
    r.pc = usNewPc;
}


/*
 *  Jump
 *
 *  Boing!  Boing!  Boing!
 *
 *  Sorry.  This is JMP, and it uses an annoying memory indexing mode,
 *  like nothing else, so I need to code it seperately here.  Icky.
 */
void Jump(HWND hWnd, unsigned char opcode)
{
    unsigned char fIndirect;
    unsigned char msb, lsb;
    unsigned short usOff;
    
    fIndirect = opcode & 0x20;  /* 5th bit is the sign! */
    
    lsb = ONEB;
    msb = TWOB;
    
    /* do the indirection... */
    if(fIndirect)
    {
        usOff = lsb + (msb * 0x100);
        lsb = GetRam(usOff);
        msb = GetRam(US(usOff+1));
    }
    
    /* merge the msb and lsb */
    usOff = lsb + (msb * 0x100);
    
    MOVEPC; /* This is here to add ticks */

    /* reset the pc */
    r.pc = usOff;
}


/*
 *  Break
 *
 *  Software interrupt or trap.  There's only one on the 6502,
 *  and it's a mess of stack smashing and register bopping.
 */
void Break(HWND hWnd, unsigned char opcode)
{
    unsigned char msb, lsb, sb;
    
    /* pc adds 2 and then pushed to stack... */
    r.pc+=2;
    msb = HIBYTE(r.pc);
    lsb = LOBYTE(r.pc);
    
    /* Okay, push the PC onto the stack... */
    SetRam(US((STACK_START+r.sp)), msb);
    r.sp--;
    SetRam(US((STACK_START+r.sp)), lsb);
    r.sp--;
       

    /* set the break bit, then push the status flags onto the stack */
    sb = r.s;
    sb = sb | BREAKB;   /* set the break bit */
    SetRam(US((STACK_START+r.sp)), sb);
    r.sp--;


    /* and jump to the point stored at FFFE,FFFF */
    msb = GetRam(0xFFFF);
    lsb = GetRam(0xFFFE);
    
    MOVEPC; /* add ticks */
    r.pc = lsb + (msb * 0x100);
    
    /* and disable hardware interrupts! */
    r.s = r.s | INTRB;
}


/*
 *  Return From Interrupt
 *
 *  Now, what WAS I DOING?
 *
 *  Reload the PC, load registers, 
 */
void ReturnInter(HWND hWnd, unsigned char opcode)
{
    unsigned char msb, lsb, sb;
    
    /* pop the status byte, and pc from the stack */
    r.sp++;
    sb = GetRam(US(STACK_START+r.sp));
    r.sp++;
    lsb = GetRam(US(STACK_START+r.sp));
    r.sp++;
    msb = GetRam(US(STACK_START+r.sp));
    
    /* reinstall sb */    
    r.s = sb;
    
    MOVEPC; /* tick counter... */
    
    /* set the PC back where it goes */
    r.pc = lsb + (msb * 0x100);
}


