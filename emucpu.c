/****************************************************************************

    PROGRAM: emucpu.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    CpuWindow - handles all processing for the CPU window.
    
    COMMENTS:

    This is the neato CPU control window.  It's going to be pretty ugly, so
    I can get on with writing instructions.
    
****************************************************************************/
#include "emu.h" 
           

DLGPROC dpCpuProc;
static HICON hMemIcon;


// local prototypes
long cpu_initdialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
void cpu_command(HWND hWnd, int id, HWND hWndCtl, UINT uiNotify);
void cpu_toggle(HWND hWnd, int id, short byte);
void cpu_setFlags(HWND hWnd, short compare);
short cpu_getCurrentFlags(HWND hWnd);

void SetDialogButtons(HWND hWnd, int run);
void ClearDlgValues(HWND hWnd);
void LoadDlgValues(HWND hWnd);
void SetDlgValues(HWND hWnd);
void InstallDlgValues(HWND hWnd);

//
//  Create a new memory window.
//
BOOL CreateCPUWindow(void)
{
	if(IsWindow(hWndCPU))
	{
		dPrintf(L"Open an existing CPU Window\r\n");
		SetFocus(hWndCPU); 
		ShowWindow(hWndCPU, SW_RESTORE);
		return 1;
	}
	else
	{
		dPrintf(L"Create a NEW CPU window\r\n");
//		dpCpuProc = MakeProcInstance((FARPROC)CpuWndProc, hInst);
		dpCpuProc = (DLGPROC)CpuWndProc;
		hWndCPU = CreateDialog(hInst, L"CPU_DLG", NULL, dpCpuProc);
		hMemIcon = LoadIcon(hInst, MAKEINTRESOURCE(CPU_ICON));
	}
	return 1;
}

//
//	clean up nicely
//
void DestroyCPUWindow(void)
{
	if(IsWindow(hWndCPU))
	{
		DestroyWindow(hWndCPU);
		hWndCPU = NULL;
		FreeProcInstance(dpCpuProc);
		DestroyIcon(hMemIcon);
	}
}

long EXPORT CALLBACK CpuWndProc(hWnd, message, wParam, lParam)
HWND hWnd;               /* window handle of the dialog box */
unsigned message;        /* type of message                 */
WPARAM wParam;             /* message-specific information    */
LPARAM lParam;
{
	LONG lRet = TRUE;
	static int fIcon = 0;
	PAINTSTRUCT ps;
	HDC hDc;
    
    switch (message)
    {

		case WM_COMMAND:
		return HANDLE_WM_COMMAND(hWnd, wParam, lParam, cpu_command);
			
		case WM_INITDIALOG:
		return cpu_initdialog(hWnd, wParam, lParam);

		case WM_CLOSE:
		PostMessage(hWndMain, WM_EATCPU, 0, 0L);
		return 0;
		
		case WM_SIZE:
		if(wParam == SIZE_MINIMIZED)
			fIcon = 1;
		else
			fIcon = 0;
		if(fIcon)
			dPrintf(L"we are minimized now\r\n");
		else
			dPrintf(L"we are not minimized now\r\n");

		UpdateWindow(hWnd);
		break;	

		case WM_PAINT:
		hDc = BeginPaint(hWnd, &ps);
		if(fIcon)
		{
			DrawIcon(hDc, 0, 0, hMemIcon);
		}
		else
		{
			// let the dialog code cope somehow
		}
		EndPaint(hWnd, &ps);
		break;		

		case WM_CPUREFRESH:
	    SetDlgValues(hWnd);
	    UpdateWindow(hWnd);
	    return 0;
		
        default:                  /* Passes it on if unproccessed    */
        return 0;
    }

    return (lRet);
}


void cpu_command(HWND hWnd, int id, HWND hWndCtl, UINT uiNotify)
{
	switch(id)
	{
		case IDHELP:	// get help for the CPU window
        DialogBox(hInst, L"NotDone", hWnd, NotDone);
        return;
            
        case IDB_HALT: // halt the 6502
        r.halt = HALT;
	    SetDlgValues(hWnd);
	    UpdateWindow(hWnd);
		return;
			
		case IDB_GO:	// start the 6502 again
		r.halt = RUN;
	    SetDlgValues(hWnd);
	    UpdateWindow(hWnd);
		return;
			
		case IDB_SINGLE: // take one tender step
		r.halt = SINGLE;
		Call6502(hWndMain);
		r.halt = HALT;
	    SetDlgValues(hWnd);
	    UpdateWindow(hWnd);
		break;
			
		case IDB_REFRESH: // reload current values
	    SetDlgValues(hWnd);
	    UpdateWindow(hWnd);
		return;

		case IDB_SET:		// give current to 6502			
		InstallDlgValues(hWnd);
	    SetDlgValues(hWnd);
	    UpdateWindow(hWnd);
		return;
		
		case IDC_SIGNC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, SIGNB);
		}
		return;

		case IDC_OVRC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, OVERFB);
		}
		return;

		case IDC_UNUSEDC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, UNUSEDB);
		}
		return;

		case IDC_BREAKC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, BREAKB);
		}
		return;

		case IDC_DECC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, DECIB);
		}
		return;

		case IDC_INTC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, INTRB);
		}
		return;

		case IDC_ZEROC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, ZEROB);
		}
		return;

		case IDC_CARRYC:
		if(uiNotify == BN_CLICKED)
		{
			cpu_toggle(hWnd, id, CARRYB);
		}
		return;

		case IDC_HEXVAL:	// as they type, toggle the checkboxes! (snicker)
		if(uiNotify == EN_CHANGE)
		{
			cpu_setFlags(hWnd, cpu_getCurrentFlags(hWnd));
		}
		return;
	}
	return;
}


/*
 *	get and make valid a flag value from the text box.
 */
short cpu_getCurrentFlags(HWND hWnd)
{
	WCHAR szTemp[21];
	WCHAR *stop;
	long tempVal;
	short flags;

	/* load and conver the current window contents to a number */
	GetDlgItemText(hWnd, IDC_HEXVAL, szTemp,	20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xff)
	{
		tempVal = 0xff;
	}
	flags = (short)tempVal;
    return flags;
}

/*
 *	Set the checkboxes as they type.
 */
void cpu_toggle(HWND hWnd, int id, short byte)
{
	WCHAR szTemp[21];
	short flags;
	
	flags = cpu_getCurrentFlags(hWnd);
	
	if(flags & byte)	// if that bin is set
	{
		/* then unset it and uncheck the check box */
		flags = flags & (~byte);
		SendDlgItemMessage(hWnd, id, BM_SETCHECK, 0, 0L);
	}
	else	// the bit is not set
	{
		// set it and check it's box
		flags = flags | byte;
		SendDlgItemMessage(hWnd, id, BM_SETCHECK, 1, 0L);
	}

	/* set hex field */
	wsprintf(szTemp, L"%02x", flags);
	SetDlgItemText(hWnd, IDC_HEXVAL, szTemp);

}

//
//	handle initalizeing of the dialog
//
long cpu_initdialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    SetDlgValues(hWnd);
	SetFocus(GetDlgItem(hWnd, IDB_GO));
	return 0L;
}

//
//	Refresh all the things in the dialog
//
void SetDlgValues(HWND hWnd)
{
	if(r.halt == RUN)	// cpu running full-speed
	{
		ClearDlgValues(hWnd);
		SetDialogButtons(hWnd, 0);
	}
	else
	{
		LoadDlgValues(hWnd);
		SetDialogButtons(hWnd, 1);
	}
}

//
//	Assume the dialog is open - load it with values
//
void LoadDlgValues(HWND hWnd)
{
	WCHAR szTemp[21];

	/* set all the text fields */
	wsprintf(szTemp, L"%02x", r.s);
	SetDlgItemText(hWnd, IDC_HEXVAL, szTemp);
	
	wsprintf(szTemp, L"%02x", r.a);
	SetDlgItemText(hWnd, IDC_EDITA, szTemp);
	
	wsprintf(szTemp, L"%02x", r.x);
	SetDlgItemText(hWnd, IDC_EDITX, szTemp);
	
	wsprintf(szTemp, L"%02x", r.y);
	SetDlgItemText(hWnd, IDC_EDITY, szTemp);
	
	wsprintf(szTemp, L"%02x", r.pc);
	SetDlgItemText(hWnd, IDC_EDITIP, szTemp);
	
	wsprintf(szTemp, L"%02x", r.sp);
	SetDlgItemText(hWnd, IDC_EDITSP, szTemp);

	cpu_setFlags(hWnd, r.s);

	CheckDlgButton(hWnd, IDC_HALTX,	!(r.halt));
} 

void cpu_setFlags(HWND hWnd, short compare)
{
#define ZAP(x) ((x) == 0 ? 0 : 1)
	
	/* set bits */
	CheckDlgButton(hWnd, IDC_SIGNC, 	ZAP(compare & SIGNB));
	CheckDlgButton(hWnd, IDC_OVRC,  	ZAP(compare & OVERFB));
	CheckDlgButton(hWnd, IDC_UNUSEDC,	ZAP(compare & UNUSEDB));
	CheckDlgButton(hWnd, IDC_BREAKC,	ZAP(compare & BREAKB));
	CheckDlgButton(hWnd, IDC_DECC,		ZAP(compare & DECIB));
	CheckDlgButton(hWnd, IDC_INTC,		ZAP(compare & INTRB));
	CheckDlgButton(hWnd, IDC_ZEROC,		ZAP(compare & ZEROB));
	CheckDlgButton(hWnd, IDC_CARRYC,	ZAP(compare & CARRYB));
}

//
//	Assume the dialog is busy
//
void ClearDlgValues(HWND hWnd)
{
	WCHAR szTemp[21];

	/* set all the text fields */
	wsprintf(szTemp, L"--", r.s);
	
	SetDlgItemText(hWnd, IDC_HEXVAL, szTemp);
	SetDlgItemText(hWnd, IDC_EDITA, szTemp);
	SetDlgItemText(hWnd, IDC_EDITX, szTemp);
	SetDlgItemText(hWnd, IDC_EDITY, szTemp);
	SetDlgItemText(hWnd, IDC_EDITIP, szTemp);
	SetDlgItemText(hWnd, IDC_EDITSP, szTemp);
	
	/* set bits */
	CheckDlgButton(hWnd, IDC_SIGNC, 	2);
	CheckDlgButton(hWnd, IDC_OVRC,  	2);
	CheckDlgButton(hWnd, IDC_UNUSEDC,	2);
	CheckDlgButton(hWnd, IDC_BREAKC,	2);
	CheckDlgButton(hWnd, IDC_DECC,		2);
	CheckDlgButton(hWnd, IDC_INTC,		2);
	CheckDlgButton(hWnd, IDC_ZEROC,		2);
	CheckDlgButton(hWnd, IDC_CARRYC,	2);

	CheckDlgButton(hWnd, IDC_HALTX,	!(r.halt));
}

//
//	Set the buttons to ok or grey
//
void SetDialogButtons(HWND hWnd, int run)
{
	EnableWindow(GetDlgItem(hWnd, IDB_HALT), !run);
	
	EnableWindow(GetDlgItem(hWnd, IDB_GO), run);
	EnableWindow(GetDlgItem(hWnd, IDB_SINGLE), run);
	EnableWindow(GetDlgItem(hWnd, IDB_SET), run);
	EnableWindow(GetDlgItem(hWnd, IDB_REFRESH), run);
	EnableWindow(GetDlgItem(hWnd, IDHELP), run);
	
}


//
//	Copy values from dialog to the CPU it's self
//
void InstallDlgValues(HWND hWnd)
{
	WCHAR szTemp[21];
	WCHAR *stop;
	long tempVal;

	GetDlgItemText(hWnd, IDC_HEXVAL, szTemp,	20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xff)
	{
		MessageBox(hWnd, L"Warning: Flags value too large, truncating.", L"6502 CPU", MB_OK);
	}
	r.s = (char)tempVal;
	
	GetDlgItemText(hWnd, IDC_EDITA, szTemp,		20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xff)
	{
		MessageBox(hWnd, L"Warning: Accumulator value too large, truncating.", L"6502 CPU", MB_OK);
	}
	r.a = (char)tempVal;

	GetDlgItemText(hWnd, IDC_EDITX, szTemp,		20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xff)
	{
		MessageBox(hWnd, L"Warning: X register value too large, truncating.", L"6502 CPU", MB_OK);
	}
	r.x = (char)tempVal;

	GetDlgItemText(hWnd, IDC_EDITY, szTemp,		20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xff)
	{
		MessageBox(hWnd, L"Warning: Y register value too large, truncating.", L"6502 CPU", MB_OK);
	}
	r.y = (char)tempVal;

	GetDlgItemText(hWnd, IDC_EDITIP, szTemp,	20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xffff)
	{
		MessageBox(hWnd, L"Warning: IP value too large, truncating.", L"6502 CPU", MB_OK);
	}
	r.pc = (short)tempVal;

	GetDlgItemText(hWnd, IDC_EDITSP, szTemp,	20);
	tempVal = wcstol(szTemp, &stop, 16);
	if(tempVal > 0xff)
	{
		MessageBox(hWnd, L"Warning: SP value too large, truncating.", L"6502 CPU", MB_OK);
	}
	r.sp = (char)tempVal;
}
