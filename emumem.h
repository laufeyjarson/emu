/****************************************************************************

    PROGRAM: memory.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

    structures and things for the memory window.
        
****************************************************************************/

/* These are the things each memory window needs to know to draw it's self.
   These are stored in a handle in the windowHandle of the memory hWnd, so
   all we really need is the hWnd of a memory window to get to everything.  */
struct memData
{
    unsigned short sStart;       /* First byte of memory window             */
    unsigned short sPos;         /* position of cursor                      */
    unsigned short sWidth;       /* width in pixels of win                  */
    unsigned short sHeight;      /* height of win in pixels                 */
    unsigned short sFontWidth;   /* width of the widest char in the font    */
    unsigned short sFontHeight;  /* height of a charchter                   */
    unsigned short sWindowID;    /* ID of a window in the menu              */
};

/* The memory window is 72 charchters wide. */
#define MEM_WIDTH 72

/* it'll be at least 10 rows high. */
#define MEM_MIN_ROWS 10

#define MEM_PIXELS (MEM_WIDTH * lpInfo->sFontWidth)

/* constants for memory window word */
#define MEM_HDL   0

/* cheezy macros to get and set this data for emumem.c's functions. */
#define FREEINFO GlobalUnlock(hInfo)
#define GETINFO hInfo=(HANDLE)GetWindowHandle(hWnd, MEM_HDL);lpInfo=(struct memData *)GlobalLock(hInfo)

