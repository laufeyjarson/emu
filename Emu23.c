/****************************************************************************

    PROGRAM: emu23.c

    PURPOSE: 6502 emulator engine in 2023

    FUNCTIONS:

    WinMain() - calls initialization function, processes message loop
    InitApplication() - initializes window data and registers window
    InitInstance() - saves instance handle and creates main window
    MainWndProc() - processes messages
    About() - processes messages for "About" dialog box

    COMMENTS:

    This program is intended to emulate a 6502 processor with it's local 64K
    of data space.  It will also have a neat UI to allow editing of memory,
    and interruption of execution.

****************************************************************************/
#include "emu.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HACCEL hAccelTable;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

void DoAboutBox(void);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NotDone(HWND, UINT, WPARAM, LPARAM);

void DoMessageHandling(MSG* lpmsg);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EMU23, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMU23));

    static int quitApp = 0;
    MSG msg;

    /* Acquire and dispatch messages until a WM_QUIT message is received. */
    while (quitApp == 0)
    {
        // If the processor is running then run full speed.
        if (r.halt == RUN)
        {
            if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) != 0)
            {
                if (msg.message == WM_QUIT)
                {
                    quitApp = 1;
                }
                DoMessageHandling((MSG *) & msg);
            }
        }
        else    /* system is halted, be polite. */
        {
            quitApp = ! (BOOL)GetMessageW(&msg, NULL, 0, 0);
            DoMessageHandling((MSG *)(& msg));
        }

        // Run instructions for a while
        int rep;
        for (rep = 0; r.halt == RUN && rep < IDLE_INSTR; rep++)
        {
            Call6502(hWndMain);
        }
    }

    // Free the system resources.  Unlocks it as well.
    CloseMemWindows();
    DestroyCPUWindow();
    vStopVideo();
    FreeMemory();
    DebugQuit;

    return (int) msg.wParam;
}

/*
 *  This handles the message filters; we need (sometimes) to be careful of
 *  a modeless dialog box and to handle accelerators for all windows.
 */
void DoMessageHandling(MSG * lpmsg)
{
    if (!IsWindow(lpmsg->hwnd))      // getting rips; filter here
        return;

    if (hWndCPU != NULL)
    {
        if (IsDialogMessage(hWndCPU, lpmsg) == 0)
        {
            if (TranslateAccelerator(lpmsg->hwnd, hAccelTable, lpmsg) == 0)
            {
                TranslateMessage(lpmsg);
                DispatchMessage(lpmsg);
            }
        }
    }
    else
    {
        if (TranslateAccelerator(lpmsg->hwnd, hAccelTable, lpmsg) == 0)
        {
            TranslateMessage(lpmsg);
            DispatchMessage(lpmsg);
        }
    }
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wcex.style = CS_BYTEALIGNCLIENT;      /* Class style(s).                    */
    wcex.lpfnWndProc = MainWndProc;       /* Function to retrieve messages for  */
                                        /* windows of this class.             */
    wcex.cbClsExtra = 0;                  /* No per-class extra data.           */
    wcex.cbWndExtra = 0;                  /* No per-window extra data.          */
    wcex.hInstance = hInstance;           /* Application that owns the class.   */
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON_ID));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = L"EMUMENU";   /* Name of menu resource in .RC file. */
    wcex.lpszClassName = szWindowClass; /* Name used in call to CreateWindow. */
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON_ID));

    /* Register the window class and return success/failure code. */
    if (RegisterClassExW(&wcex) == 0)
    {
        MessageBoxW(NULL, L"Cannot create main window class", L"Initalization failure",
            MB_ICONASTERISK);
        return 0;
    }

    /* Fill in window class structure with parameters that describe the       */
    /* memory window.                                                         */

    wcex.style = CS_DBLCLKS;                       /* Class style(s).                    */
    wcex.lpfnWndProc = MemoryWndProc;     /* Function to retrieve messages for  */
                                        /* windows of this class.             */
    wcex.cbClsExtra = 0;                  /* No per-class extra data.           */
    wcex.cbWndExtra = sizeof(HANDLE);     /* a handle to a structure.           */
    wcex.hInstance = hInstance;           /* Application that owns the class.   */
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(APP_MEM_ID));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;        /* Name of menu resource in .RC file. */
    wcex.lpszClassName = L"EmuMemWClass"; /* Name used in call to CreateWindow. */

    /* Register the window class and return success/failure code. */
    if (RegisterClassExW(&wcex) == 0)
    {
        MessageBox(NULL, L"Cannot create memory window class", L"Initalization failure",
            MB_ICONASTERISK);
        return 0;
    }
    return 1;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    // call a macro to start the cool debugging code
    DebugInit;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    // save that hWnd, it's useful to know
    hWndMain = hWnd;

    DebugStartMenu(hWnd);

   /* Make the window visible; update its client area; and return "success" */
//		don't do this here; let the video deal with sizing and making show...
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);

    // Allocate and initalize the system RAM area
    if (InitMemory() == 0)
    {
        MessageBox(NULL, L"Cannot allocate system memory", L"Initalization Failure",
            MB_ICONASTERISK);
        return 0;
    }

    // setup the 6502 machine
    Init6502(hWnd);

    DragAcceptFiles(hWndMain, TRUE);

    vStartVideo();

    return TRUE;
}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

    WM_COMMAND    - application menu (About dialog box)
    WM_DESTROY    - destroy window

    COMMENTS:


****************************************************************************/
LRESULT CALLBACK    MainWndProc(
    HWND hWnd,                      /* window handle                 */
    UINT message,                   /* type of message               */
    WPARAM wParam,                  /* additional information        */
    LPARAM lParam                   /* additional information        */
)
{
    HDROP hDrop;

    switch (message)
    {
    case WM_COMMAND:       /* message: command from application menu */
        switch (wParam)
        {
            /* File Menu */
        case ID_FILE_NEW:
            dPrintf(L"file/new\r\n");
            FileNewMenu(hWnd);
            break;

        case ID_FILE_OPEN:
            dPrintf(L"file/open\r\n");
            FileOpenMenu(hWnd);
            break;

        case ID_FILE_SAVE:
            dPrintf(L"file/save\r\n");
            FileSaveMenu(hWnd);
            break;

        case ID_FILE_SAVEAS:
            dPrintf(L"file/save as\r\n");
            FileSaveAsMenu(hWnd);
            break;

        case ID_FILE_CLOSE:
            dPrintf(L"file/close\r\n");
            FileCloseMenu(hWnd);
            break;

        case ID_FILE_EXIT:
            dPrintf(L"file/exit: bye!\r\n");
            DestroyWindow(hWnd);
            break;

            /* Window Menu */
        case ID_WINDOW_MEMORY:
            CreateMemoryWindow();
            break;

        case ID_WINDOW_CPU:
            CreateCPUWindow();
            break;

            // close all the windows
        case ID_WINDOW_CLOSEALL:
            CloseMemWindows();
            DestroyCPUWindow();
            break;

            /* Help memu */
        case ID_HELP_CONTENTS:
            dPrintf(L"Help: contsnts\r\n");
            DialogBox(hInst, L"NotDone", hWnd, NotDone);
            break;

        case ID_HELP_SEARCH:
            dPrintf(L"help: search\r\n");
            DialogBox(hInst, L"NotDone", hWnd, NotDone);
            break;

        case IDM_ABOUT:
            DoAboutBox();
            break;

            // debug menu handler here
            DebugMenu;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
        }
        break;

    case WM_DESTROY:          /* message: window being destroyed */
        DragAcceptFiles(hWndMain, FALSE);
        PostQuitMessage(0);
        break;


#ifdef WIN32
    case WM_DROPFILES:
        hDrop = (HDROP)wParam;
        FileOpenDroppedFile(hWndMain, hDrop);
        break;
#endif

        /* force the icon to get painted */
    case WM_ICONERASEBKGND:
        DefWindowProc(hWnd, message, wParam, lParam);
        UpdateWindow(hWnd);
        return 0;

        /* The only "clean" way for a modeless dialog to die */
    case WM_EATCPU:
        DestroyCPUWindow();
        break;

    default:                  /* Passes it on if unproccessed    */
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}


/*
 *  This handles a cool about box.
 */
void DoAboutBox(void)
{
    DialogBox(hInst, L"AboutBox", hWndMain, About);
}

/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - Input received

    COMMENTS:

    No initialization is needed for this particular dialog box, but TRUE
    must be returned to Windows.

    Wait for user to click on "Ok" button, then close the dialog box.

    Notice that this function is completely unused if we are in WIN32 - the
    provided ShellAbout is used instead.

****************************************************************************/
LRESULT CALLBACK About(
    HWND hDlg,               /* window handle of the dialog box */
    unsigned message,        /* type of message                 */
    WPARAM wParam,             /* message-specific information    */
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_INITDIALOG:            /* message: initialize dialog box */
        return (TRUE);

    case WM_COMMAND:               /* message: received a command */
        if (wParam == IDOK         /* "OK" box selected?          */
            || wParam == IDCANCEL) /* System menu close command?  */
        {
            EndDialog(hDlg, TRUE); /* Exits the dialog box        */
            return (TRUE);
        }
        break;
    }
    return (FALSE);               /* Didn't process a message    */
}


/****************************************************************************

    FUNCTION: NotDone(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "NotDone" dialog box.  This is a really
              stupid function, AND it looks just like the origional About function,
              but I wanted it to be seperate - in case I added more to About.

    MESSAGES:

    Wait for user to click on "Ok" button, then close the dialog box.

****************************************************************************/
LRESULT CALLBACK NotDone(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return (TRUE);

    case WM_COMMAND:
        if (wParam == IDOK
            || wParam == IDCANCEL)
        {
            EndDialog(hDlg, TRUE);
            return (TRUE);
        }
        break;
    }
    return (FALSE);
}
