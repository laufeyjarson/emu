/****************************************************************************

    PROGRAM: emuglob.c

    PURPOSE: 6502 emulator engine globals

    FUNCTIONS:
    
    There aren't any here

    COMMENTS:

    This module is to create global variables and will use memory.
    
****************************************************************************/
#include "emu.h" 

// stupidly this is the processor's registers, out here as a struct.
struct __reg r;

HANDLE hInst;               // This instance
HWND hWndMain;              // Main window
HWND hWndCPU;               // CPU window


// debugger info
#ifdef _DEBUG
int haveDBTerminal;
#endif


// add opcodes HERE.  There must be 255 at completeion.
//  {opcode, bytes, cycles, "NMEMONIC", fnPointer},
struct __opcode instr[0x100] = {
    {   0x00,   1,  7,  "BRK",  Break       },
    BAD(0x01),
    BAD(0x02),
    BAD(0x03),
    BAD(0x04),
    BAD(0x05),
    {   0x06,   2,  5,  "ASL",  ShiftLeft   },
    BAD(0x07),
    {   0x08,   1,  3,  "PHP",  PushFlags   },
    BAD(0x09),
    {   0x0A,   1,  2,  "ASL",  ShiftLeft   },
    BAD(0x0B),
    BAD(0x0C),
    BAD(0x0D),
    {   0x0E,   3,  6,  "ASL",  ShiftLeft   },
    BAD(0x0F),

    {   0x10,   2,  2,  "BPL",  Branch      },
    BAD(0x11),
    BAD(0x12),
    BAD(0x13),
    BAD(0x14),
    BAD(0x15),
    {   0x16,   2,  6,  "ASL",  ShiftLeft   },
    BAD(0x17),
    {   0x18,   1,  2,  "CLC",  ClearCarry  },
    BAD(0x19),
    BAD(0x1A),
    BAD(0x1B),
    BAD(0x1C),
    BAD(0x1D),
    {   0x1E,   3,  7,  "ASL",  ShiftLeft   },
    BAD(0x1F),

    {   0x20,   3,  6,  "JSR",  JumpSub     },
    {   0x21,   2,  6,  "AND",  AndByte     },
    BAD(0x22),
    BAD(0x23),
    {   0x24,   2,  3,  "BIT",  BitByte     },
    {   0x25,   2,  3,  "AND",  AndByte     },
    BAD(0x26),
    BAD(0x27),
    {   0x28,   1,  4,  "PLP",  PopFlags    },
    {   0x29,   2,  2,  "AND",  AndByte     },
    BAD(0x2A),
    BAD(0x2B),
    {   0x2C,   3,  4,  "BIT",  BitByte     },
    {   0x2D,   3,  4,  "AND",  AndByte     },
    BAD(0x2E),
    BAD(0x2F),

    {   0x30,   2,  2,  "BMI",  Branch      },
    {   0x31,   2,  5,  "AND",  AndByte     },
    BAD(0x32),
    BAD(0x33),
    BAD(0x34),
    {   0x35,   2,  4,  "AND",  AndByte     },
    BAD(0x36),
    BAD(0x37),
    {   0x38,   1,  2,  "SEC",  SetCarry    },
    {   0x39,   3,  4,  "AND",  AndByte     },
    BAD(0x3A),
    BAD(0x3B),
    BAD(0x3C),
    {   0x3D,   3,  4,  "AND",  AndByte     },
    BAD(0x3E),
    BAD(0x3F),

    {   0x40,   1,  6,  "RTI",  ReturnInter },
    BAD(0x41),
    BAD(0x42),
    BAD(0x43),
    BAD(0x44),
    BAD(0x45),
    BAD(0x46),
    BAD(0x47),
    {   0x48,   1,  3,  "PHA",  PushAccum   },
    BAD(0x49),
    BAD(0x4A),
    BAD(0x4B),
    {   0x4C,   3,  3,  "JMP",  Jump        },
    BAD(0x4D),
    BAD(0x4E),
    BAD(0x4F),

    {   0x50,   2,  2,  "BVC",  Branch      },
    BAD(0x51),
    BAD(0x52),
    BAD(0x53),
    BAD(0x54),
    BAD(0x55),
    BAD(0x56),
    BAD(0x57),
    {   0x58,   1,  2,  "CLI",  ClearIntr   },
    BAD(0x59),
    BAD(0x5A),
    BAD(0x5B),
    BAD(0x5C),
    BAD(0x5D),
    BAD(0x5E),
    BAD(0x5F),

    {   0x60,   1,  6,  "RTS",  ReturnSub   },
    BAD(0x61),
    BAD(0x62),
    BAD(0x63),
    BAD(0x64),
    BAD(0x65),
    BAD(0x66),
    BAD(0x67),
    {   0x68,   1,  4,  "PLA",  PopAccum    },
    BAD(0x69),
    BAD(0x6A),
    BAD(0x6B),
    {   0x6C,   3,  5,  "JMP",  Jump        },
    BAD(0x6D),
    BAD(0x6E),
    BAD(0x6F),

    {   0x70,   2,  2,  "BVS",  Branch      },
    BAD(0x71),
    BAD(0x72),
    BAD(0x73),
    BAD(0x74),
    BAD(0x75),
    BAD(0x76),
    BAD(0x77),
    {   0x78,   1,  2,  "SEI",  SetIntr     },
    BAD(0x79),
    BAD(0x7A),
    BAD(0x7B),
    BAD(0x7C),
    BAD(0x7D),
    BAD(0x7E),
    BAD(0x7F),

    BAD(0x80),
    {   0x81,   2,  6,  "STA",  StoreA      },
    BAD(0x82),
    BAD(0x83),
    {   0x84,   2,  3,  "STY",  StoreY      },
    {   0x85,   2,  3,  "STA",  StoreA      },
    {   0x86,   2,  3,  "STX",  StoreX      },
    BAD(0x87),
    {   0x88,   1,  2,  "DEY",  DecY        },
    BAD(0x89),
    {   0x8A,   1,  2,  "TXA",  TransferReg },
    BAD(0x8B),
    {   0x8C,   3,  4,  "STY",  StoreY      },
    {   0x8D,   3,  4,  "STA",  StoreA      },
    {   0x8E,   3,  4,  "STX",  StoreX      },
    BAD(0x8F),

    {   0x90,   2,  2,  "BCC",  Branch      },
    {   0x91,   2,  6,  "STA",  StoreA      },
    BAD(0x92),
    BAD(0x93),
    {   0x94,   2,  4,  "STY",  StoreY      },
    {   0x95,   2,  4,  "STA",  StoreA      },
    {   0x96,   2,  4,  "STX",  StoreX      },
    BAD(0x97),
    {   0x98,   1,  2,  "TYA",  TransferReg },
    {   0x99,   3,  5,  "STA",  StoreA      },
    {   0x9A,   1,  2,  "TXS",  TransferReg },
    BAD(0x9B),
    BAD(0x9C),
    {   0x9D,   3,  5,  "STA",  StoreA      },
    BAD(0x9E),
    BAD(0x9F),
           
    {   0xA0,   2,  2,  "LDY",  LoadY       },
    {   0xA1,   2,  6,  "LDA",  LoadAccum   },
    {   0xA2,   2,  2,  "LDX",  LoadX       },
    BAD(0xA3),
    {   0xA4,   2,  3,  "LDY",  LoadY       },
    {   0xA5,   2,  3,  "LDA",  LoadAccum   },
    {   0xA6,   2,  3,  "LDX",  LoadX       },
    BAD(0xA7),
    {   0xA8,   1,  2,  "TAY",  TransferReg },
    {   0xA9,   2,  2,  "LDA",  LoadAccum   },
    {   0xAA,   1,  2,  "TAX",  TransferReg },
    BAD(0xAB),
    {   0xAC,   3,  4,  "LDY",  LoadY       },
    {   0xAD,   3,  4,  "LDA",  LoadAccum   },
    {   0xAE,   3,  4,  "LDX",  LoadX       },
    BAD(0xAF),

    {   0xB0,   2,  2,  "BCS",  Branch      },
    {   0xB1,   2,  5,  "LDA",  LoadAccum   },
    BAD(0xB2),
    BAD(0xB3),
    {   0xB4,   2,  4,  "LDY",  LoadY       },
    {   0xB5,   2,  4,  "LDA",  LoadAccum   },
    {   0xB6,   2,  4,  "LDX",  LoadX       },
    BAD(0xB7),
    {   0xB8,   1,  2,  "CLV",  ClearOverf  },
    {   0xB9,   3,  4,  "LDA",  LoadAccum   },
    {   0xBA,   1,  2,  "TSX",  TransferReg },
    BAD(0xBB),
    {   0xBC,   3,  4,  "LDY",  LoadY       },
    {   0xBD,   3,  4,  "LDA",  LoadAccum   },
    {   0xBE,   3,  4,  "LDX",  LoadX       },
    BAD(0xBF),

    BAD(0xC0),
    BAD(0xC1),
    BAD(0xC2),
    BAD(0xC3),
    BAD(0xC4),
    BAD(0xC5),
    {   0xC6,   2,  5,  "DEC",  DecMem      },
    BAD(0xC7),
    {   0xC8,   1,  2,  "INY",  IncY        },
    BAD(0xC9),
    {   0xCA,   1,  2,  "DEX",  DecX        },
    BAD(0xCB),
    BAD(0xCC),
    BAD(0xCD),
    {   0xCE,   3,  6,  "DEC",  DecMem      },
    BAD(0xCF),

    {   0xD0,   2,  2,  "BNE",  Branch      },
    BAD(0xD1),
    BAD(0xD2),
    BAD(0xD3),
    BAD(0xD4),
    BAD(0xD5),
    {   0xD6,   2,  6,  "DEC",  DecMem      },
    BAD(0xD7),
    {   0xD8,   1,  2,  "CLD",  ClearDec    },
    BAD(0xD9),
    BAD(0xDA),
    BAD(0xDB),
    BAD(0xDC),
    BAD(0xDD),
    {   0xDE,   3,  7,  "DEC",  DecMem      },
    BAD(0xDF),

    BAD(0xE0),
    BAD(0xE1),
    BAD(0xE2),
    BAD(0xE3),
    BAD(0xE4),
    BAD(0xE5),
    {   0xE6,   2,  5,  "INC",  IncMem      },
    BAD(0xE7),
    {   0xE8,   1,  2,  "INX",  IncX        },
    BAD(0xE9),  
    {   0xEA,   1,  2,  "NOP",  NoOp        },
    BAD(0xEB),
    BAD(0xEC),
    BAD(0x0D),
    {   0xEE,   3,  6,  "INC",  IncMem      },
    BAD(0xEF),

    {   0xF0,   2,  2,  "BEQ",  Branch      },
    BAD(0xF1),
    BAD(0xF2),
    BAD(0xF3),
    BAD(0xF4),
    BAD(0xF5),
    {   0xF6,   2,  6,  "INC",  IncMem      },
    BAD(0xF7),
    {   0xF8,   1,  2,  "SED",  SetDec      },
    BAD(0xF9),
    BAD(0xFA),
    BAD(0xFB),
    BAD(0xFC),
    BAD(0xFD),
    {   0xFE,   3,  7,  "INC",  IncMem      },
    BAD(0xFF)
    };