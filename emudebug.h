/****************************************************************************

    PROGRAM: debug.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    COMMENTS:

    Debugging header stuff, can always be included
    
****************************************************************************/

#if defined(_DEBUG)

#define WM_DEBUG_ENABLE WM_USER+0x200


#define DebugInit DEBUG_INIT()
#define DebugQuit DEBUG_QUIT()
#define DebugStartMenu(hWnd) DEBUG_START_MENU(hWnd)
#define DebugMenu case WM_DEBUG_ENABLE: DEBUG_MENU_GOT()

void DEBUG_INIT(void);
void DEBUG_QUIT(void);
void DEBUG_START_MENU(HWND);
void DEBUG_MENU_GOT(void);
void DEBUG_HALT(void);
void DEBUG_RESTORE(void);


#else

#define DebugStartMenu(hWnd) hWnd=hWnd
#define DebugInit
#define DebugQuit
#define DebugMenu

#endif

/* always prototype this */
int dPrintf(PCWSTR fmt, ...);
