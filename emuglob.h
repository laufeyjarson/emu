/****************************************************************************

    PROGRAM: enuglob.h

    PURPOSE: 6502 emulator engine

    COMMENTS:

    This contains definitions of all global variables, as extern with no
    inializer.  See emuglob.c for comments on what each one is used for.
    
****************************************************************************/


/* window handle for main window */
extern HANDLE hInst;               
extern HWND hWndMain;
extern HWND hWndCPU;

/* These are all the instructions */
extern struct __opcode instr[0x100];


#ifdef _DEBUG
extern int haveDBTerminal;
#endif
