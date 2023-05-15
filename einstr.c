/****************************************************************************

    PROGRAM: einstr.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    GetSetVal()     - get or set a value to memory based on the opcide
    Decode()        - local fn to to the ugly work.

    COMMENTS:

    This is the instruction decoder function.  Read it's comments carefully.

****************************************************************************/
#include "emu.h"

/*
 *  The 6502 has a fairly logical list of opcodes; most of them can be
 *  broken up based on the bit patterns.  This code breaks opcodes apart  
 *  returns the correct value to handle, or sets it to a value.  This means
 *  the ugly reference code has to be in place only once.
 *
 *  These addressing modes are complicated, and I do not try and fully explain
 *  them here.  Consult a 6502 book for details; I used Lance A. Levanthal's
 *  "6502 Assembly Language Programming".  See the bibliology in the info
 *  directory for full details on it.
 *
 *  This works by building a far pointer to the destination value in memory.
 *  Then, if it's a read, we read from that pointer, else we write the
 *  value passed in to us to it, and so our memory modes are in only one place.
 *
 *  Note that we now build an offset into a 64k data space, then call a GetRam
 *  or SetRam function; these handle the paging and fake out an MMU, and the
 *  processor, correctly, dosen't know about what may be going on behind
 *  the scenes.
 *
 *  The sysArray table was the hardest part; see the decode.txt in the info
 *  directory for an explanation and a bunch of my scratch work.  Note that
 *  passing an argumentless opcode to this fn returns one of the error codes;
 *  an 0xEA (nop) would fail, even though it's a perfectly valid opcode.
 */

enum MemTypes { error, izpx, zp, imm, mabs, zpy, zpx, absx, absy };
static enum MemTypes sysArray[3][8] = 
{
{   imm,    zp, error,  mabs,    error,  zpx,    error,  absx    },
{   izpx,   zp, imm,    mabs,    zpy,    zpx,    absy,   absx    },
{   imm,    zp, error,  mabs,    error,  zpy,    error,  absy    }
};


/*
 *  Decode
 *
 *  Decode an opcode into a named, hopefully useful type.
 *  Note error is valid on certian opcodes...
 */
unsigned short INLINE Decode(unsigned char opcode)
{
    enum MemTypes type;
    unsigned char mid, final;

    /* mask out the middle three bits (0001 1100 is 0x1C) and shift over to
       make a small constant value that is the access type */
    mid = ((opcode & 0x1C) >> 2);
    /* this is just the last two bits */
    final = opcode & 0x03;

    /* now get the type and parse it */
    type = sysArray[final][mid];

    return type;
}


/*
 *  GetSetVal
 *
 *  Get or set a value based on the opcode.  Uses the tables developed above.
 *
 */
unsigned char GetSetVal(unsigned char opcode, unsigned char what, unsigned char val)
{
    unsigned char lsb, msb;
    unsigned short srcOffs;

    switch(Decode(opcode))
    {
        case izpx:  // (zp, x) LDA (20,x) = load A with *( *20+x)
                    // note the cast is to force mod 256
            lsb = GetRam((unsigned char)(ONEB + r.x));
            msb = GetRam((unsigned char)(ONEB + r.x+1));
            srcOffs = (lsb+(msb*0x100));
            break;

        case zp:    // zp   LDA X = load a with addr x
            srcOffs = ONEB;
            break;

        case imm:   // immediate LDA x = load a with x
            srcOffs = r.pc + 1;
            break;

        case mabs:   // absolute LDA
                    // addresses are stored LSB first, so we fetch backwards
            srcOffs = ( ONEB + (TWOB * 0x100) );
            break;

        case zpy:   // (zp),y   lda (x),z load a with *( *x+y )
            lsb = GetRam(ONEB);
            msb = GetRam((unsigned short)(ONEB+1));
            srcOffs = ( lsb + (msb * 0x100) + r.y);
            break;

        case zpx:   // zp,x     lda 20,x is load a *(20+x)
            srcOffs = (ONEB+r.x);
            break;

        case absy:  // abs,y    lda 1133.y load a with *(3311+y)
            lsb = ONEB;
            msb = TWOB;
            srcOffs= (lsb + (msb*0x100) + r.y);
            break;

        case absx:  // abs,x    lda 1133,x load a with *(3311+x)
            lsb = ONEB;
            msb = TWOB;
            srcOffs = (lsb + (msb*0x100) + r.x);
            break;

        case error:
            dPrintf(L"opcode %d got into GetSetValues!\r\n", opcode);
            return 0;
    }

    /* either read from or write to the source byte */
    if(what == GET)
    {
        return GetRam(srcOffs);
    }
    else
    {
        SetRam(srcOffs, val);
        return 0;
    }
}


