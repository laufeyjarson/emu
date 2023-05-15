/****************************************************************************

    PROGRAM: emu.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    COMMENTS:   This is the main header file for the Apple emulator.
                It includes most other things, and has some definitions
                of it's own.  It is important and should be included in
                each EMU module.

****************************************************************************/

#pragma once

/* Someday, we'll come back and fix this.  For now, they're annoying errors.  */
#define _CRT_SECURE_NO_WARNINGS


/* Add all of the system headers here - they aren't clean and I like to keep
   the masses of #pargmas all in one place . . . */
#pragma warning( disable : 4201 4214 4001 )

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>

/* #include <stdlib.h> */
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <malloc.h>
#include <tchar.h>
#include <io.h>

#include "targetver.h"


/* These are local headers, but they are made by sloppy App Studio
   and I can't fix them every time, so they get hidden too */
#include "resource.h"
/* #include "resrc1.h" old, let it go */
#pragma warning( default: 4201 4214 4001 )

/* Our local header files */
#include "e6502.h"
#include "emuglob.h"
#include "emumem.h"
#include "emudebug.h"
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
#define GetWindowHandle(h, o)       (HANDLE)GetWindowLongPtr(h, o)
#define SetWindowHandle(h, o, v)    SetWindowLongPtr(h, o, (LONG_PTR)v)

#define GetWindowPointer(h, o)		(LPVOID)GetWindowLongPtr(h, o)
#define SetWindowPointer(h, o, v)	SetWindowLongPtr(h, o, (LONG_PTR)v)
    
/* Allow INLINE */
#define INLINE __inline

/* Allow __export */
#define EXPORT

#ifdef OLD
extern unsigned short GetRegister(unsigned short eReg);
#endif

#define GetRegister(x) (r.x)


/* Prototypes. */
/* emu.c */
extern BOOL InitApplication(HANDLE);
extern BOOL InitInstance(HINSTANCE, int);
extern LRESULT CALLBACK EXPORT MainWndProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT EXPORT CALLBACK About(HWND, unsigned, WPARAM, LPARAM);
extern LRESULT EXPORT CALLBACK NotDone(HWND, unsigned, WPARAM, LPARAM);


/* emumem.c */
extern LRESULT CALLBACK EXPORT MemoryWndProc(HWND, UINT, WPARAM, LPARAM);
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
void FileOpenDroppedFile(HWND hWnd, HDROP hDrop);
void FileSaveAsMenu(HWND hWnd);
void FileOpenMenu(HWND hWnd);
void FileCloseMenu(HWND hWnd);
void FileNewMenu(HWND hWnd);
UINT_PTR EXPORT CALLBACK FileOpenHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* cpumem.c */
short cpuSaveFile(SFTYPE FAR *lpFileInfo);
short cpuLoadFile(SFTYPE FAR *lpFileInfo, char zero);
short cpuBuildSFTYPE(SFTYPE FAR *lpFileInfo, PWSTR szFileName);

/* memchain.c */
extern BOOL InitMemory(void);
extern BOOL InitRegisters(void);
extern BOOL FreeMemory(void);
extern BOOL FreeRegisters(void);
extern unsigned short SetRegister(unsigned short eReg, unsigned short ucVal);
extern unsigned char GetRam(unsigned short usAddr);
extern unsigned char SetRam(unsigned short usAddr, BYTE ucVal);
extern short JoinMemChain(HWND hWnd, unsigned short usStart, unsigned short usEnd);
extern short QuitMemChain(HWND hWnd);
void StartUpdateCache(void);
void StopUpdateCache(void);

/* evideo.c */
extern void vStartVideo(void);
extern void vStopVideo(void);
	
