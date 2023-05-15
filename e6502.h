/****************************************************************************

    PROGRAM: e6502.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

    Structures and constants for the 6502 emulator
        
****************************************************************************/


/*
 *  This defines a single opcode, including mnenonic and cycles
 */
struct __opcode
{
    unsigned char opcode;
    unsigned char bytes;
    unsigned char cycles;
    const char *instr;
    void (*fn)(HWND, unsigned char);
};

/*
 *  This defines a CPU's registers.  Notice that I've jammed some extras
 *  here that are helpful to me.
 */
struct __reg
{
    unsigned short pc;      /* program counter                              */
    unsigned char a;        /* accumulator                                  */
    unsigned char x;        /* x- reg                                       */
    unsigned char y;        /* y-reg                                        */
    unsigned char s;        /* sign/flags bit                               */
    unsigned char sp;       /* stack pointer                                */
    unsigned char halt;     /* is the processor running?  (meta-register)   */
    unsigned long ulTicks;  /* tick count (meta-register)                   */
};

/* status for "r.halt" */
enum Halted { HALT = 0, RUN = 1, SINGLE = 2 };

enum RegName { pc, a, x, y, s, sp, halt, ticks };

/* cheezy "bad" opcode definition */
#define BAD(x) {x, 0, 0, "???", BadInstr}

/* bump up the PC the right amount (used in opcode fn's)
   also now keeps ulTicks up to date for timing         */
#define MOVEPC r.pc += instr[opcode].bytes; r.ulTicks += instr[opcode].cycles

/* Quick reference to "one byte from here" and "two bytes from here" */
#define ONEB (GetRam((unsigned short)(r.pc+1)))
#define TWOB (GetRam((unsigned short)(r.pc+2)))

#define SIGNB   0x80    /* sign bit only            */
#define OVERFB  0x40    /* overflow bit only        */
#define UNUSEDB 0x20    /* unused bit               */
#define BREAKB  0x10    /* break bit only           */
#define DECIB   0x08    /* decimal mode bit only    */
#define INTRB   0x04    /* Intr bit only            */
#define ZEROB   0x02    /* zero bit only            */
#define CARRYB  0x01    /* carry bit only           */

#define SIGNBIT(x)      if((x) & SIGNB)\
                            r.s = r.s|SIGNB;\
                        else\
                            r.s = r.s & (~SIGNB)    
                             
#define OVERFBIT(x)     if((x) > 255) \
                            r.s = r.s|OVERFB; \
                        else \
                            r.s = r.s & (~OVERFB)
                            
#define ZEROBIT(x)      if((x) == 0)\
                            r.s = r.s|ZEROB;\
                        else\
                            r.s = r.s & (~ZEROB)

#define SETCARRY        r.s = r.s | CARRYB
#define CLRCARRY        r.s = r.s & ~(CARRYB)
                                
