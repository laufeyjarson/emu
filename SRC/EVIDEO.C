/****************************************************************************

    PROGRAM: evideo.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

	This is the code to handle all of the bizarre video modes the Apple
	Ever dealt with.

	Things get scarey in here, but we do what we can to document them.

	The text screens are stored in the Apple's ram - we just calc the
	correct value, and paint that one directly to the screen.  There's no
	double buffering there.

	The hires screens, on the other hand, are kept translated into Windows
	naitve bitmaps;  When RAM is changed, the image-map is updated, and
	if visible, repainted, via a BitBlt to the real screen.  This allows
	Windows to quickly repaint the hires screens without doing hundreds
	of translations to and from Apple video.

	Note there are two video child windows defined here, the AppleTextWnd,
	and the AppleHiresWnd.  Each has a seperate WndProc,
	to do all the normal windows painting and things.

	Each of the window procs also gets the messages from the Memory Chain
	Manager to update part or all of the window.

	Note that the 80 column card is NOT currently supportedm but that we're
	thinking about it as we write this, so it should be a possible thing to
	add (although, likely as a software hack, not exactly as Apple did it)

	Note that all screen sizes and cusror pos and things are stored as an
	unsigned short, ONE BASED!  1,1 is upper left, NOT 0,0!

****************************************************************************/

#include "emu.h" 
#include "evideo.h"

#include "utils.h"

/* local prototypes */
void videoRegisterWndClasses(void);
void videoCreateTextWindows(void);
void videoDestroyTextWindows(void);
void videoUnRegisterWndClasses(void);
LRESULT CALLBACK EXPORT TextWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void videoCreateFont(void );
void videoDestroyFont(void);
void videoSizeParentWindow(void);

/* old style message fn's */
LRESULT text_paint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT text_create(HWND hWnd, WPARAM wParam, LPARAM lParam);
void text_destroy(HWND hWnd, LPARAM lParam, WPARAM wParam);
LRESULT text_memrefresh(HWND hWnd, WPARAM wParam, LPARAM lParam);


unsigned short CalcScreenFromXY(unsigned short h, unsigned short v, unsigned short usStart);
void CalcXYFromScreen(unsigned short *usRow, unsigned short *usCol, unsigned short usMem,
	unsigned short StartOfScreen);


/* module globals */
HWND ghwTextWnd[MAX_TEXT];
HFONT ghTextFont;
short gsTextHeight, gsTextWidth;
HBRUSH gahLoResBrushes[MAX_LO_COLOR];

/*
 *	vStartVideo
 *
 *	Start the video rolling, get all the windows open, and set all the
 *	latches and memory hooks.
 */
void vStartVideo(void)
{
	/* we don't know if we're first, so act as if we are... */
	videoRegisterWndClasses();

	videoCreateFont();

	videoCreateTextWindows();

	videoSizeParentWindow();
}

/*
 *	vStopVideo
 *
 *	Close all the windows, deselect all the latches, and free things
 *
 */
void vStopVideo(void)
{
	videoDestroyTextWindows();
	videoDestroyFont();
	videoUnRegisterWndClasses();
}


/*
 * videoRegisterWndClasses
 *
 *	Register all the needed window classes, and make things good to
 *	create the windows.
 *
 */
void videoRegisterWndClasses(void)
{
    WNDCLASS  wc;
	int i;

    /* Fill in window class structure with parameters that describe the       */
    /* text window.                                                           */

    wc.style = CS_BYTEALIGNCLIENT;      /* Class style(s).                    */
    wc.lpfnWndProc = TextWndProc;       /* Function to retrieve messages for  */
                                        /* windows of this class.             */
    wc.cbClsExtra = 0;                  /* No per-class extra data.           */
    wc.cbWndExtra = sizeof(LPVOID);     /* Room in each instance for a handle */
    wc.hInstance = hInst;               /* Application that owns the class.   */
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName =  NULL;   /* Name of menu resource in .RC file. */
    wc.lpszClassName = "EmuVTextClass"; /* Name used in call to CreateWindow. */

    /* Register the window class and return success/failure code. */
    if (RegisterClass(&wc) == 0)
    {
        MessageBox(NULL, "Cannot create text window class", "Initalization failure",
            MB_ICONASTERISK);
        return;    
    }        

	// TODO: register a lores and a hires class

	for(i = 0; i < MAX_LO_COLOR; i++)
	{
		gahLoResBrushes[i] = CreateSolidBrush(crLoResColors[i]);
	}

}


/*
 *	videoCreateTextWindows
 *
 *	Create and initalize all the text windows on the system.
 */
void videoCreateTextWindows(void)
{
	HWND hwNew;
	short sDest;
	DWORD dwStyle;

#ifdef _DEBUG
	dwStyle = WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS/*|WS_BORDER*/;
#else
	dwStyle = WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
#endif

	for(sDest = 1; sDest <= MAX_TEXT; sDest++)
	{
		hwNew = CreateWindow("EmuVTextClass",
				"Text Window",
				dwStyle,
				0,	/* x pos */
				0,	/* y pos */
				10,	/* x width - will correct inside when we know corect font size */
				10,	/* y height */
				hWndMain,
				NULL,
				hInst,
				((LPVOID)(&(stTextTab[sDest])))
				);

		ghwTextWnd[sDest-1] = hwNew;
	}
}

void videoDestroyTextWindows(void)
{
	short sDest;

	for(sDest = 1; sDest <= MAX_TEXT; sDest++)
	{
		DestroyWindow(ghwTextWnd[sDest-1]);
	}
}

void videoUnRegisterWndClasses(void)
{
	int i;

	UnregisterClass("EmuVTextClass", hInst);
	for(i = 0; i < MAX_LO_COLOR; i++)
	{
		DeleteObject(gahLoResBrushes[i]);
	}
}


/*
 *	TextWndProc
 *
 *	This is the handler for the TextWindow.
 *
 */
LRESULT CALLBACK EXPORT TextWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:       /* message: command from application menu */
            switch(wParam)
            {
                default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break;

		case WM_CREATE:
			return text_create(hWnd, wParam, lParam);

        case WM_DESTROY:          /* message: window being destroyed */
			text_destroy(hWnd, wParam, lParam);
            break;

		case WM_PAINT:
			return text_paint(hWnd, wParam, lParam);

        case WM_MEMREFRESH:
            return text_memrefresh(hWnd, wParam, lParam);

        default:                  /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

/*
 *	text_create
 *
 *	create a text window in the main window.
 */
LRESULT text_create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT lpcs;

	struct stTextVideoTab FAR *lpVideo;

	lpcs = (LPCREATESTRUCT)lParam;
	SetWindowPointer(hWnd, 0, lpcs->lpCreateParams);

	GETVIDEOINFO;


	// also, be real sneaky and tie us into the Memory Chain...
    JoinMemChain(hWnd, (unsigned short)(lpVideo->usStartAt), 
    		(unsigned short)(lpVideo->usEndAt));

	// move us to where we need to be
	MoveWindow(hWnd, 0, 0, (gsTextWidth * lpVideo->usXsize) +1, 
		(gsTextHeight * lpVideo->usYsize)+1, FALSE);
	ShowWindow(hWnd, SW_RESTORE);



	return 0;
}

/*
 *	text_destroy
 *
 *	clean up this text window nicely
 */
void text_destroy(HWND hWnd, LPARAM lParam, WPARAM wParam)
{
	/* tell the mem chain we are leaving... */
    QuitMemChain(hWnd);
}

/*
 *	text_paint
 *
 *	paint the video window
 */
LRESULT text_paint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	short min, max;
	short left, right;
	HDC hDC;
	PAINTSTRUCT ps;
    COLORREF crOld, crColor;
    COLORREF crBkOld, crBk;
	short x, y;
	unsigned char ucVal;
	HFONT hOldF;
	unsigned short usX, usY;
	unsigned short usTextStart;
//	static unsigned short usTextStartTab[] = { 0, 20, 25 };
	static unsigned short usTextStartTab[] = { 25 , 25, 25 };

	struct stTextVideoTab *lpVideo;
	GETVIDEOINFO;

    /* initalization of calculated constants and setup */
    hDC = BeginPaint(hWnd, &ps);

    crColor = GetSysColor(COLOR_WINDOWTEXT);
    crOld = SetTextColor(hDC, crColor);
    crBk = GetSysColor(COLOR_WINDOW);
    crBkOld = GetBkColor(hDC)        ;
    SetBkColor(hDC, crBk);
	hOldF = SelectObject(hDC, ghTextFont);
    
    /* calculate the area we are going to paint */
    min = (ps.rcPaint.top / gsTextHeight) - 1;
    if(min < 1) min = 1;
            
    max = (ps.rcPaint.bottom / gsTextHeight) + 1;
    if(max > (short)(lpVideo->usYsize))	max = lpVideo->usYsize;

	left = (ps.rcPaint.left / gsTextWidth) - 1;
	if(left < 1) left = 1;

	right = (ps.rcPaint.right / gsTextWidth) + 1;
	if(right > lpVideo->usXsize) right = lpVideo->usXsize;
	

	/* starting line of text; all else is graphics */
	usTextStart = usTextStartTab[lpVideo->usTextMode];

	/* sit in a little loop and get the byte and draw */
	for(y = min; y <= max; y++)
	{
		for(x = left; x <= right; x++)
		{
			ucVal = GetRam(CalcScreenFromXY(x, y, lpVideo->usStartAt));
			usX = ((x-1)*gsTextWidth);		/* upper left X pos for this char */
			usY = ((y-1)*gsTextHeight);		


			if(y <= usTextStart)	/* render a line of two lo-res blocks */
			{
				RECT rcDraw;
				unsigned short usColor;

				/* rect and pen index for top block */
				rcDraw.top = usY;
				rcDraw.left = usX;
				rcDraw.bottom = usY+ (gsTextHeight/2)+1;
				rcDraw.right = usX + gsTextWidth+1;
				/* upper four bits, then move to lower */
				usColor = ((ucVal & 0xF0)/0x10);

				FillRect(hDC, &rcDraw, gahLoResBrushes[usColor]);

				/* slide the rect down, get a new pen, paint */
				rcDraw.top = rcDraw.bottom;
				rcDraw.bottom+= (gsTextHeight/2);
				/* just the lower four bits */
				usColor = (ucVal & 0x0F);

				FillRect(hDC, &rcDraw, gahLoResBrushes[usColor]);
			}
			else /* text mode! */
			{
				/* render text mode */
				TextOut(hDC, usX, usY, (LPCSTR)&ucVal, 1);
			}
		}
	}


    SetTextColor(hDC, crOld);
    SetBkColor(hDC, crBkOld);
    SelectObject(hDC, hOldF);
    EndPaint(hWnd, &ps);

    return 0L;
}


/*
 *	text_memrefresh
 *
 *	refresh the text screen as text changes on the screen....
 */
LRESULT text_memrefresh(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    unsigned short usStart, usEnd;
	unsigned short usX, usY;
	RECT rc;

	struct stTextVideoTab *lpVideo;
	GETVIDEOINFO;

    usStart = LOWORD(lParam);
    usEnd = HIWORD(lParam);
	
	if(usStart == usEnd)	// one byte from the CPU - fast update case
	{
		CalcXYFromScreen(&usY, &usX, usStart, lpVideo->usStartAt);
		rc.top = (usY-1) * gsTextHeight;
		rc.left = (usX-1) * gsTextWidth;
		rc.bottom = rc.top+gsTextHeight;
		rc.right = rc.left+gsTextWidth;
		InvalidateRect(hWnd, &rc, TRUE);
	}
	else					// multi byte update; ok to be sloppy, as the user
	{						// has undoubtedly caused this refresh (memory window or load)
		InvalidateRect(hWnd, NULL, TRUE);
	}
	UpdateWindow(hWnd);

	return 0L;
}

/*
 *	videoCreateFont
 *
 *	Create the font and initalize the globals for all the text screens
 *
 */
void videoCreateFont(void )
{
    TEXTMETRIC tm;
    HDC hDC;
	LOGFONT lgf;
	HFONT hOldF;

	lgf.lfHeight = TEXT_HEIGHT;
	lgf.lfWidth = TEXT_WIDTH;
	lgf.lfEscapement = 0;
	lgf.lfOrientation = 0;
	lgf.lfWeight = 0;
	lgf.lfItalic = 0;
	lgf.lfUnderline = 0;
	lgf.lfStrikeOut = 0;
	lgf.lfCharSet = 0;
	lgf.lfOutPrecision = 0;
	lgf.lfClipPrecision = 0;
	lgf.lfQuality = 0;
	lgf.lfPitchAndFamily = FF_MODERN|FIXED_PITCH;
	strcpy(lgf.lfFaceName, "Courier New");


	ghTextFont = CreateFontIndirect(&lgf);
	if(ghTextFont == NULL)
	{
		MessageBox(hWndMain, "The Apple // Emulator requires Courier New.", "Error", MB_OK);
		DestroyWindow(hWndMain);
	}

    hDC = GetDC(hWndMain);  /* cheat and use the main window */
    hOldF = (HFONT) SelectObject(hDC, (HGDIOBJ)ghTextFont);
    GetTextMetrics(hDC, &tm);
    SelectObject(hDC, (HGDIOBJ) hOldF);
    ReleaseDC(hWndMain, hDC);
            
    gsTextWidth = (short)tm.tmMaxCharWidth;
    gsTextHeight = (short)tm.tmHeight + (short)tm.tmExternalLeading;
}

/*
 *	videoDestroyFont
 *
 *	Clean out the video font
 */
void videoDestroyFont(void)
{
	DeleteObject(ghTextFont);
}

/*
 *	CalcScreenFromXY
 *
 *	This always returns relative to the page start offset passed in.
 */
unsigned short CalcScreenFromXY(unsigned short h, unsigned short v, unsigned short usStart)
{
	unsigned short usRet;
	usRet = ((128 * v) + h - (984 * ((unsigned short)((v - 1) / 8))) + 895);

	usRet = usRet - 1024;	/* now an offset from the start of screen */
	usRet = usRet + usStart; /* and now an offset from the passed page... */

	return usRet;
}

/*
 *	CalcSYFromScreen
 *
 *	This returns a location from an X and a Y and a page...
 *
 *	Make sure it's ok for us to fill in usRow and usCol...
 */
void CalcXYFromScreen(unsigned short *usRow, unsigned short *usCol, unsigned short usMem,
	unsigned short StartOfScreen)
{
	register unsigned short usChunk;
	register unsigned short usOffs;
	register unsigned short usChOffs;
	register unsigned short usPart;
	register unsigned short usThird;

	/* The Chunk of the screen is the interleave id, 0-7 */
	usChunk = ( ((unsigned short)(usMem / 128)) - 8);

	/* The Offs is the offset from the start of the screen */
	usOffs = usMem - StartOfScreen;

	/* The ChOffs is the offset in ram of the start of the chunk. */
	usChOffs = usChunk * 128;

	/* Part is the offset into the chunk... */
	usPart = usOffs - usChOffs;

	/* Third is the third of the screen we are on */
	usThird = (unsigned short)(usPart / 40);

	/* The row is the third times 8 (8 rows per third) plus the row on that third... */
	*usRow = (usThird * 8) + (usChunk + 1);

	/* The column is the offset into the part minus the start of the part */
	*usCol = (usPart - (usThird * 40)) +1;
}


/*
 *	videoSizeParentWindow
 *
 *	Adjust the main window to something understandable.
 */
void videoSizeParentWindow(void)
{
	HWND hWnd;
	RECT rcNew;

	struct stTextVideoTab *lpVideo;

	hWnd = ghwTextWnd[0];
	GETVIDEOINFO;


	rcNew.left = 0;
	rcNew.top = 0;
	rcNew.right = (gsTextWidth * lpVideo->usXsize);
	rcNew.bottom = (gsTextHeight * lpVideo->usYsize);

	/* size the puppy */
	SetClientRect(hWndMain, &rcNew);

	ShowWindow(hWndMain, SW_SHOW);
}


