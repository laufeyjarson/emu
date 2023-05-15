/****************************************************************************

    PROGRAM: e6502.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    DEBUG_INIT()            init the debug engine
    DEBUG_QUIT()            quit the debug engine
    DEBUG_PRINTF(fmt, ...)  printf to the debug port!
    
    COMMENTS:

    This contains debugging code, and informational junk that will fall out
    the serial terminal.  Later versions may add support for the herc as
    well as a better cpu view.

    To make the debug module happen, you need to either set an environmental
    vairable called "CPUDEBUG" equal to any value, or be running Debug Windows.
    You can also add a -DFORCEDEBUG to the command line to force the code on.
    Once the code in on, status messages will go to the debug terminal (or
    to DB/WIN if it's loaded) if under WIN16, or to a Console Window if
    you are using the CONSOLE_DEBUGGER version.

    Notice that the 32-bit version adds a "console" type window with this
    information in it, so that we can be happy even if there is no terminal,
    and if there is no Debugger running.  CONSOLE_DEBUGGER lets us do this with no
    hard work, so we do.
    
    Also note that this module is the most affected by WIN16/CONSOLE_DEBUGGER changes,
    and #ifdef is everywhere.  Also, statics local to this module control
    the behavior of the debug engine, so we get some ugly globals in this
    module alone.
    
****************************************************************************/
#include "emu.h"

#ifdef _DEBUG

#ifdef WIN32
#ifndef _MAC
#ifndef UNDER_CE
#define CONSOLE_DEBUGGER
#endif
#endif
#endif

/*
 *  Variables; status, state, and a hConsole for the CONSOLE_DEBUGGER version.
 */
#ifdef CONSOLE_DEBUGGER
HANDLE hConsole;
#endif
static int weHaveDebugger;
static int gotMenuAdded = 0;
static HMENU hMenuDebug;

/*
 *  Initalize the debugger.  It'll only run if it's forced, or if it is sure there
 *  is a debug monitor.  (IE: Debug Windows)
 */
void DEBUG_INIT(void)
{
    int debugWin;
    
    // If there is debug kernels, run
    debugWin = GetSystemMetrics(SM_DEBUG);
    
    // If there is an env variable, you're forced
    if(getenv("CPUDEBUG") != NULL)
    {
        debugWin = 1;
    }

    // if there's a compiler switch, you're forced.
#ifdef FORCEDEBUG
    debugWin = 1;
#endif
    
#ifdef CONSOLE_DEBUGGER
    if(debugWin)
    {
        if(AllocConsole() == FALSE)
        {
            debugWin = FALSE;
        }
        else
        {
            SetConsoleTitle(L"6502 Debugging info");
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        }
    }
#endif
    // remember if we run
    haveDBTerminal = debugWin;
    weHaveDebugger = debugWin;
    dPrintf(L"Debugging module started\r\n");
}



/*
 *  Halt the debugger; stop output, hide the window.
 */
void DEBUG_HALT(void)
{
    dPrintf(L"debugger halted\r\n");
    if(haveDBTerminal == 0)
        return;

    haveDBTerminal = 0;

#ifdef CONSOLE_DEBUGGER
    FreeConsole();
#endif
}


/*
 *  This is called via macro in the WM_COMMAND message of the main window.
 *  It halts or restores the debug system, and toggles the window checkmark.
 */
void DEBUG_MENU_GOT()
{
    DWORD isChecked;

    isChecked = CheckMenuItem(hMenuDebug, 0, MF_BYPOSITION|MF_CHECKED);

    if(isChecked == 0xFFFFFFFF)
    {
        dPrintf(L"meaningless menu check\r\n");
    }

    if(isChecked == MF_CHECKED)
    {
        CheckMenuItem(hMenuDebug, 0, MF_BYPOSITION|MF_UNCHECKED);
        DEBUG_HALT();
    }
    else
    {
        DEBUG_RESTORE();
    }


}


/*
 *  This will startup the debug system again; creates the console if need be
 */
void DEBUG_RESTORE(void)
{
    if(weHaveDebugger == 0 || haveDBTerminal == 1)
        return;

    haveDBTerminal = 1;
#ifdef CONSOLE_DEBUGGER
    if(AllocConsole() == FALSE)
    {
        hConsole = NULL;
        haveDBTerminal = 0;
        weHaveDebugger = 0;
    }
    else
    {
        SetConsoleTitle(L"6502 Debugging info");
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
#endif
    dPrintf(L"debugger re-started\r\n");
}


/*
 *  This hooks in and adds a new menu with an enabled check to the menu bar.
 *  It needs a handle to the hWnd that contains the menu.
 */
void DEBUG_START_MENU(HWND hWnd)
{
    HMENU hMenuMain;

    hMenuDebug = CreateMenu();
    if(hMenuDebug == NULL)
    {
        dPrintf(L"Can't create new menu, none added\r\n");
    }
    AppendMenu(hMenuDebug, MF_STRING|MF_CHECKED, WM_DEBUG_ENABLE, L"&Enabled");

    hMenuMain = GetMenu(hWnd);
    if(hMenuMain == NULL)
    {
        dPrintf(L"No menu in app window - no debug controls added\r\n");
        return;
    }
    AppendMenu(hMenuMain, MF_POPUP, (UINT_PTR)hMenuDebug, L"&Debug");

    gotMenuAdded = 1;
}


/*
 *  Cleans up after the debugger, closing the debug window if need be, and
 *  discarding the menus we added.
 */
void DEBUG_QUIT(void)
{
    dPrintf(L"debugging module stopped\r\n");
#ifdef CONSOLE_DEBUGGER
    FreeConsole();
#endif

    return;
}


/*
 *  dPrintf - the debug printf.  Prints a header, then the message.  Note the strage
 *  indenting of the header - my terminal is fried so that you can't read the first five
 *  charachters . . . so, you all get indents.
 */
int dPrintf(PCWSTR fmt, ...)
{
    va_list vlist;
    static WCHAR pBuff[4096];
    static char mbcsStr[4096];
    int rtn;
#ifdef CONSOLE_DEBUGGER
    DWORD dwWrite;
#endif
    
    // if no debugging, blow
    if(haveDBTerminal == 0)
        return 0;
        
    // do the printf
    va_start(vlist, fmt);
    rtn = wvsprintf(pBuff, fmt, vlist);
    va_end(vlist);
    
#ifndef CONSOLE_DEBUGGER
    // dump it out
    OutputDebugString(L"     EMU6502: ");
    OutputDebugString(pBuff);
#else
    WriteFile(hConsole, "EMU6502: ", 9, &dwWrite, NULL);
    WideCharToMultiByte(CP_ACP, 0, pBuff, -1, mbcsStr, sizeof(mbcsStr), NULL, NULL);
    WriteFile(hConsole, mbcsStr, (DWORD)strlen(mbcsStr), &dwWrite, NULL);
#endif

    return rtn;
}

#else /* retail */

// stubbed dPrintf
int dPrintf(char *fmt, ...)
{
    return 0;
}

#endif
