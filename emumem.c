/****************************************************************************

    PROGRAM: emumem.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

    This is the code to handle the drawing and manipulation of the "Memory"
    window that is avalible for the 6502's memory space.
    
****************************************************************************/
#include "emu.h" 


/* These are local functions. */
void GetSystemFontSizes(short far *width, short far *height);

/* message cracker fn's */
void mem_vscroll(HWND hWhd, HWND hCtrl, UINT wCode, int nPos);
void mem_ldown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT uiFlags);
void mem_keydown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT cFlags);
void mem_mousewheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys);

/* old message fn's */ 
long mem_paint(HWND hWnd, WPARAM wParam, LPARAM lParam);
long mem_create(HWND hWnd, WPARAM wParam, LPARAM lParam);
long mem_getmmi(HWND hWnd, WPARAM wParam, LPARAM lParam);
long mem_size(HWND hWnd, WPARAM wParam, LPARAM lParam);
long mem_memrefresh(HWND hWnd, WPARAM wParam, LPARAM lParam);

long EXPORT CALLBACK mem_editProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam);
short mem_validateHex(HWND hWnd, unsigned int uiControl, unsigned int uiError);
void mem_addHex(HWND hWnd, int uiControl, unsigned short sStartAt);
char getHexVal(WCHAR **str);
void MarkSelection(HWND hWnd, unsigned int uiControl, WCHAR *pBase, WCHAR *pStart, WCHAR *pEnd);


#define SB_MOUSEWHEEL SB_ENDSCROLL+2

/*
 *  This is the main window handler for the memory windows that will
 *  be floating around.
 */
LRESULT CALLBACK EXPORT MemoryWndProc(
    HWND hWnd,                      /* window handle                 */
    UINT message,                   /* type of message               */
    WPARAM wParam,                  /* additional information        */
    LPARAM lParam                   /* additional information        */
)
{
    HANDLE hInfo;
    struct memData far *lpInfo;

    /* Note the GETINFO and FREEINFO macros */
    switch (message)
    {
        case WM_CREATE:
            return mem_create(hWnd, wParam, lParam);
                    
        case WM_PAINT:
            return mem_paint(hWnd, wParam, lParam);
            
        /* Message cracker's are my friend. */
        HANDLE_MSG(hWnd, WM_VSCROLL, mem_vscroll);
                        
        HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, mem_ldown);
        
        HANDLE_MSG(hWnd, WM_KEYDOWN, mem_keydown);

        HANDLE_MSG(hWnd, WM_MOUSEWHEEL, mem_mousewheel);
        
        case WM_COMMAND:       /* message: command from application menu */
            switch(wParam)
            {
                default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break;

        case WM_DESTROY:
            GETINFO;            
            FREEINFO;
            QuitMemChain(hWnd);
            GlobalFree(hInfo);
        return 0;
        
        /* when we are an icon, force a paint */
        case WM_ICONERASEBKGND:
            DefWindowProc(hWnd, message, wParam, lParam);
            UpdateWindow(hWnd);
            return 0;

        case WM_SIZE:
            return mem_size(hWnd, wParam, lParam);

        case WM_GETMINMAXINFO:
            return mem_getmmi(hWnd, wParam, lParam);
        
        case WM_MEMREFRESH:
            return mem_memrefresh(hWnd, wParam, lParam);

        default:                  /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0L);
}


/*
 *  mem_char
 *
 *  Handle the keyboard input we may get as of now, just scrolling
 */
void mem_keydown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT cFlags)
{
    if(fDown == 0)
        return;
        
    switch(vk)
    {
        case VK_UP:
        mem_vscroll(hWnd, NULL, SB_LINEUP,   0);
        break;
        
        case VK_DOWN:
        mem_vscroll(hWnd, NULL, SB_LINEDOWN, 0);
        break;
        
        case VK_PRIOR:
        mem_vscroll(hWnd, NULL, SB_PAGEUP,   0);
        break;
        
        case VK_NEXT:
        mem_vscroll(hWnd, NULL, SB_PAGEDOWN, 0);
        break;
        
        case VK_HOME:
        mem_vscroll(hWnd, NULL, SB_TOP,      0);
        break;
        
        case VK_END:
        mem_vscroll(hWnd, NULL, SB_BOTTOM,   0);
        break;
        
        default:
        break;
    }
}

/*
 *  handle the MEMREFRESH message that we get
 */ 
long mem_memrefresh(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HANDLE hInfo;
    struct memData far *lpInfo;
    unsigned short usStart, usEnd;
    unsigned short usOffs;
    unsigned short usRow;
    int iStartPix;
    RECT rcUpdate;
    unsigned short usEndOffs, usEndRow;
    
    GETINFO;
    
    usStart = LOWORD(lParam);
    usEnd = HIWORD(lParam);
    
    /* calculate what row the start byte is on */
    usOffs = usStart - lpInfo->sStart;
    usRow = usOffs/16;

    /* calculate the end row */
    usEndOffs = usEnd - lpInfo->sStart;
    usEndRow = usEndOffs/16;
    
    /* Now, calculate a rect to update, and paint it */
    iStartPix = usRow * lpInfo->sFontHeight;
    rcUpdate.left = 0;
    rcUpdate.right = lpInfo->sWidth;
    rcUpdate.top = iStartPix;
    /* paint to the BOTTOM of the end row... */
    rcUpdate.bottom = (usEndRow * lpInfo->sFontHeight) + lpInfo->sFontHeight;
    
    InvalidateRect(hWnd, (LPRECT)&rcUpdate, TRUE);
    UpdateWindow(hWnd);

    FREEINFO;
    
    return 0L;
}


/*
 *  handle left button clicks
 */
void mem_ldown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT uiFlags)
{
    HANDLE hInfo;
    struct memData far *lpInfo;

    int iRows, iCurr, iEnd;
    int iForceAll = 0;
    int iRowOn, iColOn;
    long lByteOn;
    
    DLGPROC lpfnEditProc;

    if(!fDoubleClick)   /* don't do squat if it's only a click */
        return;
    
    GETINFO;

    /* calculate a bunch of helpful constants */
    iRows = (int) ((unsigned)lpInfo->sHeight / (unsigned)lpInfo->sFontHeight); 
    iRows --;
    iCurr = (int) ((unsigned)lpInfo->sStart/16);
    iEnd = 4096 - iRows -1;
    iRowOn = y / lpInfo->sFontHeight;
    iColOn = x / lpInfo->sFontWidth;
    
    /* Now, turn those arrays of columns into bytes */
    lByteOn = -1L;
    if(iColOn >=8 && iColOn <= 15)
    {
        lByteOn = (long)(iColOn - 8) /2 ;
    }
    else if(iColOn >= 17 && iColOn <= 24)
    {
        lByteOn = (long)(iColOn -17) /2;
        lByteOn +=4;
    }
    else if(iColOn >= 28 && iColOn <= 35)
    {
        lByteOn = (long)(iColOn -26) /2;
        lByteOn +=7;
    }
    else if(iColOn >= 37 && iColOn <= 44)
    {
        lByteOn = (long)(iColOn -37) /2;
        lByteOn += 12;
    }
    else
    {
        MessageBeep(MB_ICONEXCLAMATION);
        FREEINFO;
        return;
    }
    
    lByteOn += (long) (lpInfo->sStart + (16 * iRowOn));

    lpfnEditProc = (DLGPROC) MakeProcInstance((FARPROC)mem_editProc, hInst);
    DialogBoxParam(hInst, L"EditMemDlg", hWnd, lpfnEditProc, (LPARAM)lByteOn);
    FreeProcInstance((FARPROC) lpfnEditProc);

    FREEINFO;
}



/*
 *  dialog fn for editing memory
 *
 *  This is pretty simple - we can help, cancel, or OK.  on OK we verify and sometimes
 *  close the dialog.  Notice this uses a slightly strange error notification.
 */
long EXPORT CALLBACK mem_editProc(HWND hDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
    static unsigned short sEditByte;
    WCHAR szTemp[21];
    RECT reParent;
        
    switch (message)
    {
        case WM_INITDIALOG:            /* message: initialize dialog box */
        sEditByte = (unsigned short)lParam;
        wsprintf(szTemp, L"%04X", sEditByte);
        SetDlgItemText(hDlg, IDC_LOCATION, szTemp);
        SetFocus(GetDlgItem(hDlg, IDC_EDITMEM));
        return 0;

        case WM_COMMAND:               
        switch(wParam)
        {
            case IDCANCEL:    /* cancel or close  */
            EndDialog(hDlg, FALSE); /* Exits the dialog box        */
            return TRUE;
            
            case IDOK:      /* ok; accept values */
            if(mem_validateHex(hDlg, IDC_EDITMEM, IDC_ERROR))
            {
                mem_addHex(hDlg, IDC_EDITMEM, sEditByte);
                GetClientRect(GetParent(hDlg), &reParent);  /* paint it all  */
                InvalidateRect(GetParent(hDlg), &reParent, TRUE);
                EndDialog(hDlg, TRUE); /* Exits the dialog box        */
            }
            return TRUE;
        }
        break;
    }
    return (FALSE);               /* Didn't process a message    */
}


#define SIZE_HEX 4095
static WCHAR szHexBuff[SIZE_HEX+1];  /* a handy big buffer */

/*
 *  Validate Hex values in an edit control are ok
 *  zero fails, nonzero (1) means OK.
 *
 *  Watch this; a walk through the string, and then a return or more
 *  string depending on the byte.  It's a big state machine at work.
 */
short mem_validateHex(HWND hWnd, unsigned int uiControl, unsigned int uiError)
{
    WCHAR *pCh;
    short sCount;
    short fInVal;
    WCHAR *pStart;
    
    memset(szHexBuff, 0, SIZE_HEX);
    GetDlgItemText(hWnd, uiControl, szHexBuff, SIZE_HEX);

    sCount = 0;
    fInVal = 0;
    pStart = &(szHexBuff[0]);
    
    for(pCh = &(szHexBuff[0]); *pCh != '\0'; pCh++)
    {
        if(fInVal)              /* if we're in a value          */
        {
            if(isxdigit(*pCh))  /* and it's another charachter  */
            {
                sCount++;
                if(sCount > 2)  /* if there's too many, blow    */
                {
                    MessageBeep(MB_ICONEXCLAMATION);
                    SetDlgItemText(hWnd, uiError, L"There are too many digits here");
                    MarkSelection(hWnd, uiControl, &(szHexBuff[0]), pStart, pCh);
                    return 0;
                }
            }
            else if(isspace(*pCh))  /* this is a space or a seperator   */
            {
                if(sCount != 2)     /* if there aren't enough, blow */
                {
                    MessageBeep(MB_ICONEXCLAMATION);
                    SetDlgItemText(hWnd, uiError, L"There are not enough digits here");
                    MarkSelection(hWnd, uiControl, &(szHexBuff[0]), pStart, pCh);
                    return 0;
                }
                else    /* gosh, it must still be ok */
                {
                    pStart = pCh;
                    sCount = 0;
                    fInVal = 0;
                }
            }
            else    /* This is a bogus digit */
            {
                MessageBeep(MB_ICONEXCLAMATION);
                SetDlgItemText(hWnd, uiError, L"This is not a hex number");
                MarkSelection(hWnd, uiControl, &(szHexBuff[0]), pStart, pCh+1);
                return 0;
            }
        }
        else        /* we're busy skipping whitespaces */
        {
            if(isxdigit(*pCh))  /* start of a new digit */
            {
                fInVal = 1;
                pStart = pCh;
                sCount = 1;
            }
            else if(isspace(*pCh))  /* extra spaces are ok too */
            {
                fInVal = 0;
                pStart = pCh;
                sCount = 0;
            }
            else            /* bad byres are not ok, ever.  Blow. */
            {
                MessageBeep(MB_ICONEXCLAMATION);
                SetDlgItemText(hWnd, uiError, L"This is not a hex number");
                MarkSelection(hWnd, uiControl, &(szHexBuff[0]), pStart, pCh+1);
                return 0;
            }
        }
    }
    
    /* check for a short last byte */
    if(fInVal && sCount != 2)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        SetDlgItemText(hWnd, uiError, L"There are not enough digits here");
        MarkSelection(hWnd, uiControl, &(szHexBuff[0]), pStart, pCh);
        return 0;
    }
    return 1;
}


/*
 *  Mark part of an edit control as bad, based on two poitners, an hWnd, and a control ID
 *
 *  Depends on having a text field, and three pointers.  Has win/win32 specific things.
 */
void MarkSelection(HWND hWnd, unsigned int uiControl, WCHAR *pBase, WCHAR *pStart, WCHAR *pEnd)
{
    long sStartOffs;
    long sEndOffs;
    
    /* always show to the end of a token or the string */
    while(*pEnd != '\0' && !isspace(*pEnd))
        pEnd++;

    sStartOffs = (long)(pStart-pBase);
    sEndOffs = (long)(pEnd-pBase);
    
    SetFocus(GetDlgItem(hWnd, IDC_EDITMEM));

    SendDlgItemMessage(hWnd, uiControl, EM_SETSEL, (WPARAM)sStartOffs, (LPARAM)sEndOffs);
    SendDlgItemMessage(hWnd, uiControl, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0);
}


/*
 *  Step through the string, adding bytes
 *
 *  wants the hWnd, the control to get the bytes from, and the starting mem locaion
 */
void mem_addHex(HWND hWnd, int uiControl, unsigned short sStartAt)
{
    char val;
    WCHAR *pStr;
    unsigned short sCurr;

    memset(szHexBuff, 0, SIZE_HEX);
    GetDlgItemText(hWnd, uiControl, szHexBuff, SIZE_HEX);
    pStr = &(szHexBuff[0]);
    sCurr = sStartAt;
        
    StartUpdateCache();
    while(*pStr != '\0')
    {
        while(isspace(*pStr) && *pStr != '\0')
            pStr++;
        if(*pStr != '\0')
        {
            val = getHexVal(&pStr);
            SetRam(sCurr, val);
            sCurr++;
            pStr++;
        }
    }
    StopUpdateCache();
    return;
}


/*
 *  Given a >pointer to< a string, read a hex value from it, and increment the
 *  string over the hex, returning the parsed value.
 */
char getHexVal(WCHAR **str)
{
    WCHAR *stop;
    long tempVal;

    tempVal = wcstol(*str, &stop, 16);
    if(tempVal > 0xff)
    {
        MessageBox(NULL, L"Warning: value truncated", L"6502 Memory", MB_OK);
    }
    *str = stop;    /* advance past what we read! */

    return (char)tempVal;
}



/*
 *  handle a vscroll message.
 *
 *  This is a gnarly function to scroll as smoothly as can be expected.
 */
void mem_vscroll(HWND hWnd, HWND hCtrl, UINT wCode, int nPos)
{
    /* bunch of variables to make things readable.*/
    HANDLE hInfo;
    struct memData far *lpInfo;

    int iRows, iScroll, iCurr, iEnd, iNewCurr;
    int iForceAll = 0;
    RECT rScroll;
    WCHAR szWinTitle[40];
    
    /* I don't use this */
    hCtrl = hCtrl;

    GETINFO;

    /* calculate a bunch of helpful constants */
    iRows = (int) ((unsigned)lpInfo->sHeight / (unsigned)lpInfo->sFontHeight); 
    iRows --;
    iCurr = (int) ((unsigned)lpInfo->sStart/16);
    iEnd = 4096 - iRows -1;

    switch(wCode)
    {
        /* All of these actually scroll the window, and set a counter
           (iScroll) of how many lines to scroll */
        case SB_BOTTOM:
            iScroll = iEnd - iCurr;
            break;
        case SB_TOP:
            iScroll = 0 - iCurr;
            break;
        case SB_LINEUP:
                iScroll = -1;
            break;
        case SB_LINEDOWN:
            iScroll = 1;
            break;  
        case SB_THUMBPOSITION:
            iScroll = (iCurr - nPos) *-1;
            break;
        case SB_PAGEUP:
            iScroll = iRows * -1;
            break;
        case SB_PAGEDOWN:
            iScroll = iRows;
            break;

        /* Custom additions, senaky me */
        case SB_MOUSEWHEEL:
            iScroll = nPos;
            break;

        /* Scrolling is too slow, so just update the title,
           so they know where they are going to scroll to.  */
        case SB_THUMBTRACK:
        wsprintf(szWinTitle, L"6502 Memory $%04X", nPos * 16);
        SetWindowText(hWnd, szWinTitle);
        FREEINFO;
        return;

        default:
            FREEINFO;
            return;
    }

    /* calculate a new current starting pos, and make sure it's valid */
    iNewCurr = iCurr + iScroll;
    
    if(iNewCurr > iEnd) /* reality check: more than FFFF? */
    {
        iNewCurr = iEnd;
        iForceAll = 1;
    }
    
    if(iNewCurr < 0) /* reality check: before 0000? */
    {
        iNewCurr = 0;
        iForceAll = 1;
    }
    
    if(iNewCurr == iCurr)   /* If we don't move, blow for speed */
    {
        FREEINFO;
        return;
    }

    /* move the thumb, calculate an address, and move the title string */
    SetScrollPos(hWnd, SB_VERT, iNewCurr, TRUE);
    lpInfo->sStart = iNewCurr * 16;

    wsprintf(szWinTitle, L"6502 Memory $%04X", lpInfo->sStart);
    SetWindowText(hWnd, szWinTitle);

    /* Change the area we are watching */
    JoinMemChain(hWnd, (unsigned short)(lpInfo->sStart), (unsigned short)(lpInfo->sStart + (16 * iRows)));
    

    /* if we are repainting all, paint and blow.  Note that this should not happen
       often, but it can speed things up when it does.  This is really covering
       sloppy calculations with brute force, but this is more robust    */
    if(iForceAll == 1 || iScroll == 0)
    {
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        FREEINFO;
        return;
    }

    /* Now, scroll the client area up or down */
    GetClientRect(hWnd, (RECT far *)&rScroll);

    /* only scroll whole lines */
    rScroll.bottom = (((short)((rScroll.bottom/(unsigned)lpInfo->sFontHeight)))
                    * (unsigned)lpInfo->sFontHeight);

    /* now scroll the screen! */
    ScrollWindow(hWnd, 0, (iScroll * (unsigned)lpInfo->sFontHeight) * -1,
        (RECT far *)&rScroll, (RECT far *)&rScroll);

    /* and make a paint message */
    UpdateWindow(hWnd);

    FREEINFO;
    return;
}   

/*
 *  Handle the scroll wheel up and down messages by translating them
 *  to vscroll calls
 */
void mem_mousewheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
    int clicks = zDelta / WHEEL_DELTA;
    dPrintf(L"Scrolling %d clicks which was %d\r\n", clicks, zDelta);
    if (clicks != 0) {
        mem_vscroll(hWnd, NULL, SB_MOUSEWHEEL, clicks*-1);
    }
}

/*
 *  Paint the memory window, with current values from system memory,
 *  at the correct locaiton.
 */
long mem_paint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    /* buncha variables */
    HANDLE hInfo;
    struct memData far *lpInfo;
    HDC hDC;
    int row, min, max, col;
    HGLOBAL hTemp;
    WCHAR *lpTemp;
    WCHAR szVal[5];
    HFONT hSysF, hOldF;
    PAINTSTRUCT ps;
    unsigned char ucVal;
    COLORREF crOld, crColor;
    COLORREF crBkOld, crBk;
    unsigned short usOffs;
                        
    /* initalization of calculated constants and setup */
    hDC = BeginPaint(hWnd, &ps);
    GETINFO;

    /* we need some scratch space; allocste it neatly */
    hTemp = GlobalAlloc(GHND, (MEM_WIDTH+5)*sizeof(WCHAR));
    if(hTemp == NULL)
    {
        EndPaint(hWnd, &ps);
        return 1L;
    }
    lpTemp = GlobalLock(hTemp);
            
    /* get system font, and colors, select them into the hDC */
    hSysF = (HFONT) GetStockObject(SYSTEM_FIXED_FONT);
    hOldF = SelectObject(hDC, hSysF);

    crColor = GetSysColor(COLOR_WINDOWTEXT);
    crOld = SetTextColor(hDC, crColor);
    crBk = GetSysColor(COLOR_WINDOW);
    crBkOld = GetBkColor(hDC)        ;
    SetBkColor(hDC, crBk);
    
    /* calculate the area we are going to paint */
    min = (ps.rcPaint.top / lpInfo->sHeight) -1;
    if(min < 0) min = 0;
            
    max = (ps.rcPaint.bottom / lpInfo->sFontHeight) + 1;

    if(max > (short)(lpInfo->sHeight / lpInfo->sFontHeight))
        max = lpInfo->sHeight / lpInfo->sFontHeight;
            
    usOffs = lpInfo->sStart;
            
    /* for each row of text that's in our paint rectangle
       (max and min rows, calculated above) build a string to
       paint on the window, in hex, then paint it  */
    for(row = 0; row < max; row++)
    {
        /* Initalize the row header. */
        wsprintf(lpTemp, L"  %04.04X: ", (lpInfo->sStart + (row*16)) );
                
        /* Add on the hex data bytes. */
        for(col = 0; col < 16; col++)
        {                          
            ucVal = GetRam(usOffs);
                    
            wsprintf(szVal, L"%02.02X", ucVal);
            lstrcat(lpTemp, szVal);
                    
            /* insert spaces and dashes for readability */
            switch(col)
            {
                case 3:
                case 11:
                    lstrcat(lpTemp, L" ");
                break;
                        
                case 7:
                    lstrcat(lpTemp, L" - ");
                    break;
                            
                default:
                break;                             
            }
            usOffs++;
        }
                
        /*  reset the offset pointer */
        usOffs -=16;

        /* Add the divider */
        lstrcat(lpTemp, L" | ");
                
        /* Add on the ASCII data bytes. */
        for(col = 0; col < 16; col++)
        {
            ucVal = GetRam(usOffs);
            wsprintf(szVal, L"%c", isprint(ucVal) ? ucVal
                : '.');
            lstrcat(lpTemp, szVal);
                    
            /* insert spaces and dashes for readability */
            switch(col)
            {
                case 3:
                case 11:
                    lstrcat(lpTemp, L" ");
                break;
                        
                case 7:
                    lstrcat(lpTemp, L" - ");
                    break;
                            
                default:
                break;                             
            }
            usOffs++;
        }

        /* spew this at the window */
        TextOut(hDC, 1, row * lpInfo->sFontHeight, lpTemp, 
            lstrlen(lpTemp));

    }
            
    /*  unlock, free, cleanup, fix the DC, and blow */
    GlobalUnlock(hTemp);
    GlobalFree(hTemp);
            
    SetTextColor(hDC, crOld);
    SetBkColor(hDC, crBkOld);
    SelectObject(hDC, hOldF);
    FREEINFO;
    EndPaint(hWnd, &ps);
    
    return 0L;
}

            
/* 
 *  Handle the creation of a memory window.  remember that there can be many
 *  of thes, as many as the user wants.
 */
long mem_create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HANDLE hInfo;
    struct memData far *lpInfo;
            
    dPrintf(L"Create a new memory window\r\n");

    hInfo = GlobalAlloc(GHND, sizeof(struct memData));
    if(hInfo == NULL)
        return -1;
            
    SetWindowHandle(hWnd, 0, hInfo);
    lpInfo = (struct memData far *)GlobalLock(hInfo);
    lpInfo->sStart = 0;
    lpInfo->sPos = 0;
            
    GetSystemFontSizes(&(lpInfo->sFontWidth), &(lpInfo->sFontHeight));

    FREEINFO;
    return 0;
}



/*
 *  This function changes the sizing info in the window's info structure.
 */
long mem_size(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HANDLE hInfo;
    struct memData far *lpInfo;
    int iRows; /* rows on window */
    RECT cRect;

    GETINFO;                               

    /* save x and y of window */
    lpInfo->sWidth = LOWORD(lParam);
    lpInfo->sHeight = HIWORD(lParam);

    /* calc rows, and fix scroll bar */
    iRows = (int) ((unsigned)lpInfo->sHeight / (unsigned)lpInfo->sFontHeight); 
    iRows --;
    SetScrollRange(hWnd, SB_VERT, 0, 4096-iRows, FALSE);

    /* Get hooked into the memory chain update method */
    JoinMemChain(hWnd, (unsigned short)(lpInfo->sStart), (unsigned short)(lpInfo->sStart + (16 * iRows)));

    if(LOWORD(lParam) != (unsigned)MEM_PIXELS)
    {
        SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 
            MAKELONG(MEM_PIXELS, lpInfo->sHeight) );
    }

    GetClientRect(hWnd, (RECT far *)&cRect);
    InvalidateRect(hWnd, (RECT far *)&cRect, FALSE);
    UpdateWindow(hWnd);  /* repaint */

    FREEINFO;
    return 0L;
}

/*
 *  This handles a WM_GETMINMAXINFO message.  This occurs before the 
 *  window's creation, so we need to be careful not to rely on the hWnd.
 *  If we have a valid hWnd, then we use the data cached there, else we go
 *  strange system gyrations to get font sizes.
 *
 *  we need font sizes to calculate width of the window.
 *
 */
long mem_getmmi(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HANDLE hInfo;
    struct memData far *lpInfo;
    MINMAXINFO FAR *lpmmi;
    
    short maxWidth, minHeight;
    short fntWidth, fntHeight;

    lpmmi = (MINMAXINFO FAR *)lParam;

    hInfo=(HANDLE)GetWindowHandle(hWnd, MEM_HDL);
    /* if we don't have a window yet, get wierd with the system,
       otherwise use the cached value and free that up. */
    if(hInfo == NULL)
    {
        GetSystemFontSizes((short far *)&fntWidth, (short far *)&fntHeight);
    }
    else
    {
        lpInfo=(struct memData far *)GlobalLock(hInfo);
        fntWidth = lpInfo->sFontWidth;
        fntHeight = lpInfo->sFontHeight;
        FREEINFO;
    }

    /* get the pointers and widths ready */
    maxWidth =  fntWidth * (MEM_WIDTH + 3);
    minHeight = fntHeight * MEM_MIN_ROWS;
    
    /* Windows will have provided defaults.  Fix them. */
    lpmmi->ptMaxSize.x = maxWidth;
    lpmmi->ptMinTrackSize.x = maxWidth;
    lpmmi->ptMinTrackSize.y = minHeight;
    lpmmi->ptMaxTrackSize.x = maxWidth;
    
    return 0L;
}



/*
 *  Load and return the SYSTEM_FONT size, using no window handle
 */
void GetSystemFontSizes(short far *width, short far *height)
{
    HFONT hSysF, hOldF;
    TEXTMETRIC tm;
    HDC hDC;

    hSysF = GetStockObject(SYSTEM_FIXED_FONT);
    hDC = GetDC(hWndMain);  /* cheat and use the main window */
    hOldF = (HFONT) SelectObject(hDC, (HGDIOBJ)hSysF);
    GetTextMetrics(hDC, &tm);
    SelectObject(hDC, (HGDIOBJ) hOldF);
    ReleaseDC(hWndMain, hDC);
            
    *width = (short)tm.tmMaxCharWidth;
    *height = (short)tm.tmHeight + (short)tm.tmExternalLeading;
}    



/*
 *  Create a new memory window.
 *
 *  Create a window, and initalize the thing for use.
 */
BOOL CreateMemoryWindow(void)
{
    HWND hWnd;
    
    hWnd = CreateWindow(
        L"EmuMemWClass",                 /* See RegisterClass() call.          */
        L"6502 Memory $0000",            /* Text for window title bar.         */
        WS_THICKFRAME|WS_CAPTION|WS_SYSMENU|WS_VSCROLL|
        WS_MINIMIZEBOX,                 /* Window style.                      */
        CW_USEDEFAULT,                  /* Default horizontal position.       */
        CW_USEDEFAULT,                  /* Default vertical position.         */
        CW_USEDEFAULT,                  /* Default width.                     */
        CW_USEDEFAULT,                  /* Default height.                    */
        NULL,                           /* Overlapped windows have no parent. */
        NULL,                           /* Use the window class menu.         */
        hInst,                          /* This instance owns this window.    */
        NULL                            /* Pointer not needed.                */
    );

    /* If window could not be created, return "failure" */
    if (!hWnd)
    {
        MessageBox(NULL, L"Cannot create memory window", L"Initalization failure",
            MB_ICONASTERISK);
        return 0;    
    }
    
    /* This is bogus - WM_SIZE fixes them later */
    SetScrollRange(hWnd, SB_VERT, 0, 4096, FALSE);
    SetScrollPos(hWnd, SB_VERT, 0, TRUE);

    /* Make the window visible; update its client area; and return "success" */
    ShowWindow(hWnd, SW_SHOWNORMAL);  /* Show the window                        */
    UpdateWindow(hWnd);          /* Sends WM_PAINT message                 */
    
    return 1;
}

/*
 *  CloseMemWindows
 *
 *  close all the open memory windows nicely before the app quits
 *  also called by Window/CloseAll
 */
void CloseMemWindows(void)
{
    HWND hWndToClose;
    
    dPrintf(L"Close all memory windows\r\n");

    while((hWndToClose = FindWindow(L"EmuMemWClass", NULL)) != NULL)
    {
        DestroyWindow(hWndToClose);
    }
    
}

