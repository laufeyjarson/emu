/****************************************************************************

    PROGRAM: emu.c

    PURPOSE: 6502 emulator engine

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
#ifdef PROFILE_ME
#include <mprof.h>
#endif

// local function prototype
void DoMessageHandling(MSG far *lpmsg);
void DoAboutBox(void);
HACCEL hAccel;           
#ifdef PROFILE_ME
char *meafile = "emulator.mea";
#endif

/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

    COMMENTS: Notice the strange message pump; this handles modeless dialogs
              and accelerators nicely, and keeps the indentation of this
              function to a minimum.

****************************************************************************/
int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HINSTANCE hInstance;                
HINSTANCE hPrevInstance;          
LPSTR lpCmdLine;                
int nCmdShow;                   
{
    MSG msg;                     
    static int quitApp = 0;
    int rep;

#ifdef PROFILE_ME
	_FEnableMeas(meafile, opTimingSwap);
#endif

    if (!hPrevInstance)          
    if (!InitApplication(hInstance)) 
        return (0);      

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nCmdShow))
        return (0);

    /* Acquire and dispatch messages until a WM_QUIT message is received. */
    while (quitApp == 0)
    {
        // If the processor is running then run full speed.
        if(r.halt == RUN)
        {
            if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
            {
                if(msg.message == WM_QUIT)
                {
                    quitApp = 1;
                }    
                DoMessageHandling((MSG far *)&msg);
            }
        }
        else    /* system is halted, be polite. */
        {
            quitApp = !(GetMessage(&msg, NULL, 0, 0));
            DoMessageHandling((MSG far *)(&msg));
        }

        // Run instructions for a while
        for(rep = 0; r.halt == RUN && rep < IDLE_INSTR; rep++)
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
    
#ifdef PROFILE_ME
	_FDisableMeas();
#endif

    return (msg.wParam);       
}


/*
 *  This handles the message filters; we need (sometimes) to be careful of
 *  a modeless dialog box and to handle accelerators for all windows.
 */
void DoMessageHandling(MSG far *lpmsg)
{
    if(!IsWindow(lpmsg->hwnd))      // getting rips; filter here
        return;
        
    if(hWndCPU != NULL)
    {
        if(IsDialogMessage(hWndCPU, lpmsg) == 0)
        {
            if(TranslateAccelerator(lpmsg->hwnd, hAccel, lpmsg) == 0)
            {
                TranslateMessage(lpmsg);    
                DispatchMessage(lpmsg);     
            }
        }
    }
    else
    {
        if(TranslateAccelerator(lpmsg->hwnd, hAccel, lpmsg) == 0)
        {
            TranslateMessage(lpmsg);    
            DispatchMessage(lpmsg);     
        }
    }
}


/****************************************************************************

    FUNCTION: InitApplication(HANDLE)

    PURPOSE: Initializes window data and registers window class

    COMMENTS:

        This function is called at initialization time only if no other
        instances of the application are running.  This function performs
        initialization tasks that can be done once for any number of running
        instances.

****************************************************************************/

BOOL InitApplication(hInstance)
HINSTANCE hInstance;                  
{
    WNDCLASS  wc;

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
    wcex.lpszMenuName =  "EmuMenu";   /* Name of menu resource in .RC file. */
    wcex.lpszClassName = "EmuWClass"; /* Name used in call to CreateWindow. */

    /* Register the window class and return success/failure code. */
    if (RegisterClass(&wc) == 0)
    {
        MessageBox(NULL, "Cannot create main window class", "Initalization failure",
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
    wcex.lpszMenuName =  NULL;        /* Name of menu resource in .RC file. */
    wcex.lpszClassName = "EmuMemWClass"; /* Name used in call to CreateWindow. */

    /* Register the window class and return success/failure code. */
    if (RegisterClass(&wc) == 0)
    {
        MessageBox(NULL, "Cannot create memory window class", "Initalization failure",
            MB_ICONASTERISK);
        return 0;    
    }        

    return 1;
}


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

    COMMENTS:

        This function is called at initialization time for every instance of
        this application.  This function performs initialization tasks that
        cannot be shared by multiple instances.

        In this case, we save the instance handle in a static variable and
        create and display the main program window.

****************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
    HINSTANCE          hInstance;          
    int             nCmdShow;           
{
    HWND            hWnd;               /* Main window handle.                */
    char            szWindowName[128];

    // no CPU window open by default
    hWndCPU = NULL;

    /* Save the instance handle in static variable, which will be used in  */
    /* many subsequence calls from this application to Windows.            */
    hInst = hInstance;

    // call a macro to start the cool debugging code
    DebugInit;

    // load my keyboard accelerators
    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    if(LoadString(hInst, IDS_APPWINDOW, szWindowName, sizeof(szWindowName)) == 0)
    {
        wsprintf(szWindowName, "6502 Emulator");
    }

    /* Create a main window for this application instance.  */
    hWnd = CreateWindow(
        "EmuWClass",                    /* See RegisterClass() call.          */
        szWindowName,                   /* Text for window title bar.         */
        WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,            /* Window style.                      */
        CW_USEDEFAULT,                  /* Default horizontal position.       */
        CW_USEDEFAULT,                  /* Default vertical position.         */
        CW_USEDEFAULT,                  			/* Sized close to what we might get...*/
        CW_USEDEFAULT,                  			/*                                    */
        NULL,                           /* Overlapped windows have no parent. */
        NULL,                           /* Use the window class menu.         */
        hInstance,                      /* This instance owns this window.    */
        NULL                            /* Pointer not needed.                */
    );

    /* If window could not be created, return "failure" */
    if (!hWnd)
    {
        MessageBox(NULL, "Cannot create main window", "Initalization failure",
            MB_ICONASTERISK);
        return 0;    
    }
    // save that hWnd, it's useful to know
    hWndMain = hWnd;
        
    DebugStartMenu(hWnd);

    /* Make the window visible; update its client area; and return "success" */
//		don't do this here; let the video deal with sizing and making show...
//    ShowWindow(hWnd, nCmdShow);  /* Show the window                        */
//    UpdateWindow(hWnd);          /* Sends WM_PAINT message                 */

    // Allocate and initalize the system RAM area
    if(InitMemory() == 0)
    {
        MessageBox(NULL, "Cannot allocate system memory", "Initalization Failure",
            MB_ICONASTERISK);
        return 0;    
    }
    
    // setup the 6502 machine
    Init6502(hWnd);

	DragAcceptFiles(hWndMain, TRUE);

	vStartVideo();

    return (TRUE);               /* Returns the value from PostQuitMessage */
}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

    WM_COMMAND    - application menu (About dialog box)
    WM_DESTROY    - destroy window

    COMMENTS:


****************************************************************************/
long CALLBACK EXPORT MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;                      /* window handle                 */
UINT message;                   /* type of message               */
WPARAM wParam;                  /* additional information        */
LPARAM lParam;                  /* additional information        */
{
	HANDLE hDrop;

    switch (message)
    {
        case WM_COMMAND:       /* message: command from application menu */
            switch(wParam)
            {
                /* File Menu */
                case ID_FILE_NEW:
                    dPrintf("file/new\r\n");
                    FileNewMenu(hWnd);
                    break;
                    
                case ID_FILE_OPEN:
                    dPrintf("file/open\r\n");
                    FileOpenMenu(hWnd);
                    break;
                    
                case ID_FILE_SAVE:
                    dPrintf("file/save\r\n");
                    FileSaveMenu(hWnd);
                    break;
                    
                case ID_FILE_SAVEAS:
                    dPrintf("file/save as\r\n");
                    FileSaveAsMenu(hWnd);
                    break;
                    
                case ID_FILE_CLOSE:
                    dPrintf("file/close\r\n");
                    FileCloseMenu(hWnd);
                    break;
                    
                case ID_FILE_EXIT:
                    dPrintf("file/exit: bye!\r\n");
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
                    dPrintf("Help: contsnts\r\n");
                    DialogBox(hInst, "NotDone", hWnd, NotDone);
                    break;

                case ID_HELP_SEARCH:
                    dPrintf("help: search\r\n");
                    DialogBox(hInst, "NotDone", hWnd, NotDone);
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
			hDrop = (HANDLE)wParam;
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
    DialogBox(hInst, "AboutBox", hWndMain, About);
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
BOOL EXPORT CALLBACK About(hDlg, message, wParam, lParam)
HWND hDlg;               /* window handle of the dialog box */
unsigned message;        /* type of message                 */
WPARAM wParam;             /* message-specific information    */
LPARAM lParam;
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
BOOL EXPORT CALLBACK NotDone(hDlg, message, wParam, lParam)
HWND hDlg;               
unsigned message;        
WPARAM wParam;             
LPARAM lParam;
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

