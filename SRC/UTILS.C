/*
 *
 *
 *	Generally useful windows kind of functions
 */

#include <windows.h>
#include "utils.h"

/*
 *	Set the size of the client area of an existing window.
 *
 *	lprect should be in CLIENT co-ordinates, ie: 0,0 and cx,cy
 *
 *	both winows must be scaled to pixels...
 */
BOOL SetClientRect(HWND hWnd, LPRECT lpRect)
{
	RECT rcMain, rcClient;
	RECT rcPad;

	GetWindowRect(hWnd, &rcMain);
	GetClientRect(hWnd, &rcClient);

	/* Joy, we now have two zero based RECTS.  One client, the other window */
	rcMain.right -= rcMain.left;
	rcMain.left = 0;

	rcMain.bottom -= rcMain.top;
	rcMain.top = 0;

	/* Subtract them and we get the NC sizes */
	rcPad.right = rcMain.right-rcClient.right;
	rcPad.bottom = rcMain.bottom-rcClient.bottom;

	/* And set the window back with the correct NC sizing... */
	return SetWindowPos(hWnd, NULL, 0, 0, lpRect->right+rcPad.right, 
			lpRect->bottom+rcPad.bottom, SWP_NOMOVE|SWP_NOZORDER);
}
