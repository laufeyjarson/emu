/****************************************************************************

    PROGRAM: emu.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    COMMENTS:   This is the main header file for the Apple emulator.
                It includes most other things, and has some definitions
                of it's own.  It is important and should be included in
                each EMU module.

****************************************************************************/

/* Add all of the system headers here - they aren't clean and I like to keep
   the masses of #pargmas all in one place . . . */
#pragma warning( disable : 4201 4214 4001 )
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>

#ifndef UNDER_CE
#include <stdio.h>
#include <direct.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>



/* These are local headers, but they are made by sloppy App Studio
   and I can't fix them every time, so they get hidden too */
#include "resource.h"
#include "resrc1.h"
#pragma warning( default: 4201 4214 4001 )

/* Our local header files */
#include "e6502.h"
#include "emuglob.h"
#include "emumem.h"
#include "debug.h"
#include "cpumem.h"
#include "lgets.h"
#include "memchain.h"
#include "emuwind.h"
#include "einstr.h"

#include "listman.h"

#ifndef CPU_ICON
#define CPU_ICON 102
#endif

/* Constants */
#define IDLE_INSTR 200

#define WM_EATCPU   (WM_USER+100)   /* message to close CPU window */
#define WM_CPUREFRESH (WM_USER+101) /* message to force cpu update */
#define WM_MEMREFRESH (WM_USER+102) /* message to force memory area update */

/* macros */
/* This is a Get/Set WindowHandle macro.  Very useful. */
#ifdef WIN32
#define GetWindowHandle(h, o)       (HANDLE)GetWindowLong(h, o)
#define SetWindowHandle(h, o, v)    SetWindowLong(h, o, (LONG)v)

#define GetWindowPointer(h, o)		(LPVOID)GetWindowLong(h, o)
#define SetWindowPointer(h, o, v)	SetWindowLong(h, o, (LONG)v)
#else
#define GetWindowHandle(h, o)       (HANDLE)GetWindowWord(h, o)
#define SetWindowHandle(h, o, v)    SetWindowWord(h, o, (WORD)v)

#define GetWindowPointer(h, o)		(LPVOID)GetWindowWord(h, o)
#define SetWindowPointer(h, o, v)	SetWindowWord(h, o, (WORD)v)
#endif
    
/* Allow INLINE */
#ifdef WIN32
#define INLINE
#else
#define INLINE __inline
#endif

/* Allow __export */
#ifdef WIN32
#define EXPORT
#else
#define EXPORT __export
#endif

#ifdef OLD
extern unsigned short GetRegister(unsigned short eReg);
#endif

#define GetRegister(x) (r.x)


/* Prototypes. */
/* emu.c */
extern int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
extern BOOL InitApplication(HANDLE);
extern BOOL InitInstance(HANDLE, int);
extern long CALLBACK EXPORT MainWndProc(HWND, UINT, WPARAM, LPARAM);
extern BOOL EXPORT CALLBACK About(HWND, unsigned, WPARAM, LPARAM);
extern BOOL EXPORT CALLBACK NotDone(HWND, unsigned, WPARAM, LPARAM);


/* memory.c */
extern long CALLBACK EXPORT MemoryWndProc(HWND, UINT, WPARAM, LPARAM);
extern BOOL CreateMemoryWindow(void);
extern void CloseMemWindows(void);


/* e6502.c */
extern void Init6502(HWND hWnd);
extern void  Call6502(HWND hWnd);
extern void BadInstr(HWND hWnd, unsigned char opcode);
extern void NoOp(HWND hWnd, unsigned char opcode);
extern void LoadAccum(HWND hWnd, unsigned char opcode);
extern void LoadX(HWND hWnd, unsigned char opcode);
extern void LoadY(HWND hWnd, unsigned char opcode);
extern void ClearCarry(HWND hWnd, unsigned char opcode);
extern void SetCarry(HWND hWnd, unsigned char opcode);
extern void ClearDec(HWND hWnd, unsigned char opcode);
extern void SetDec(HWND hWnd, unsigned char opcode);
extern void ClearOverf(HWND hWnd, unsigned char opcode);
extern void SetIntr(HWND hWnd, unsigned char opcode);
extern void ClearIntr(HWND hWnd, unsigned char opcode);
extern void TransferReg(HWND hWnd, unsigned char opcode);
extern void AndByte(HWND hWnd, unsigned char opcode);
extern void ShiftLeft(HWND hWnd, unsigned char opcode);
extern void Branch(HWND hWnd, unsigned char opcode);
extern void BitByte(HWND hWnd, unsigned char opcode);
extern void StoreA(HWND hWnd, unsigned char opcode);
extern void StoreX(HWND hWnd, unsigned char opcode);
extern void StoreY(HWND hWnd, unsigned char opcode);
extern void DecMem(HWND hWnd, unsigned char opcode);
extern void DecX(HWND hWnd, unsigned char opcode);
extern void DecY(HWND hWnd, unsigned char opcode);
extern void IncMem(HWND hWnd, unsigned char opcode);
extern void IncX(HWND hWnd, unsigned char opcode);
extern void IncY(HWND hWnd, unsigned char opcode);
extern void PushAccum(HWND hWnd, unsigned char opcode);
extern void PopAccum(HWND hWnd, unsigned char opcode);
extern void PushFlags(HWND hWnd, unsigned char opcode);
extern void PopFlags(HWND hWnd, unsigned char opcode);
extern void JumpSub(HWND hWnd, unsigned char opcode);
extern void ReturnSub(HWND hWnd, unsigned char opcode);
extern void Jump(HWND hWnd, unsigned char opcode);
extern void Break(HWND hWnd, unsigned char opcode);
extern void ReturnInter(HWND hWnd, unsigned char opcode);

/* einstr.c */
extern unsigned char GetSetVal(unsigned char opcode, unsigned char what, unsigned char val);


/* emucpu.c */
BOOL CreateCPUWindow(void);
void DestroyCPUWindow(void);
extern long EXPORT CALLBACK CpuWndProc(HWND, unsigned, WPARAM, LPARAM);

/* emufile.c */
void FileSaveMenu(HWND hWnd);
void FileOpenDroppedFile(HWND hWnd, HANDLE hDrop);
void FileSaveAsMenu(HWND hWnd);
void FileOpenMenu(HWND hWnd);
void FileCloseMenu(HWND hWnd);
void FileNewMenu(HWND hWnd);
UINT EXPORT CALLBACK FileOpenHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* cpumem.c */
short cpuSaveFile(SFTYPE FAR *lpFileInfo);
short cpuLoadFile(SFTYPE FAR *lpFileInfo, char zero);
short cpuBuildSFTYPE(SFTYPE FAR *lpFileInfo, char *szFileName);

/* memchain.c */
extern BOOL InitMemory(void);
extern BOOL InitRegisters(void);
extern BOOL FreeMemory(void);
extern BOOL FreeRegisters(void);
extern unsigned short SetRegister(unsigned short eReg, unsigned short ucVal);
extern unsigned char GetRam(unsigned short usAddr);
extern unsigned char SetRam(unsigned short usAddr, unsigned char ucVal);
extern short JoinMemChain(HWND hWnd, unsigned short usStart, unsigned short usEnd);
extern short QuitMemChain(HWND hWnd);
void StartUpdateCache(void);
void StopUpdateCache(void);

/* evideo.c */
extern void vStartVideo(void);
extern void vStopVideo(void);
	
