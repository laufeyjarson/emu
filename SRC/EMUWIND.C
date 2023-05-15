/****************************************************************************

    PROGRAM: emuwind.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

    This is the code to handle keeping track of all the windows in the
    Window menu.  A Window calls us to:
    
        Add it's self to the list
        Change it's title
        Delete it's self
        
    We handle all the menu, and if the user picks the menu.
    
****************************************************************************/
#include "emu.h" 

/* These are local functions */
void SetWindowTitle(HWND hWnd, char *lpszText);

/* These are module-local globals */
static short sNewMenuID = ID_START;
static short sNewMenuPos = WINDOW_OFFS;


/*
 *  SetWindowTitle
 *
 *
 *  Sets a Memory Window Title, and edits the Window Menu...
 */
void SetWindowTitle(HWND hWnd, char *lpszText)
{
#ifdef old
    HANDLE hInfo;
    struct memData far *lpInfo;
    short sNewID;
    short sLoop;
    char szText[129];
    
    HMENU hmMain;
    HMENU hmWindow;

    /*
     *  Get the main menu handle
     */    
    hmMain = GetMenu(GetParent(hWnd));
    if(hmMain == NULL)
    {
        dPrintf("Can't get main menu to add window\r\n");
        return;
    }
    
    hmWindow = GetSubMenu(hmMain, WINDOW_MENU);
    if(hmWindow == NULL)
    {
        dPrintf("Can't get window menu handle\r\n");
        return;
    }

    /* load window information */
    GETINFO;
    
    /* we need to choose a new ID here... */
    if(lpInfo->sWindowID == NEW_WINDOW_ID)
    {
        /* quick check: does the seperator exist? */
        if(GetMenuItemID(hmWindow, WINDOW_OFFS) != 0)        
        {
            /* if there is no separator, then we know to add one */
            /* and we also know the new menu ID... */
            AppendMenu(hmWindow, MF_SEPARATOR, 0, "");
            sNewID = WINDOW_OFFS+1;
        }
        else    /* There was no seperator, walk looking for one */
        {
            short sCount = GetMenuItemCount(hmWindow);
            short sID;
            
            sNewID = WINDOW_OFFS+1;
            
            /* walk the Window menu, looking for a free ID... */
            for(sLoop = WINDOW_OFFS; sLoop < sCount; sLoop++)
            {
                sID = GetMenuItemID(hmWindow, sLoop);
                if(sID != 0)    /* not a seperator */
                {
                    /* every time, bump up sNewID, until we miss it */
                    if(sLoop-ID_START == sNewID)
                    {
                        sNewID++;
                    }
                }
            }
        }        
        /* So, we know what ID we are.  Add us to the END */
        wsprintf(szText, "Memory %d @ %s", sNewID, lpszText);
        AppendMenu(hmWindow, MF_ENABLED|MF_STRING, sNewID+ID_START, szText);

        /* save our ID */
        lpInfo->sWindowID = sNewID;

        /* we're done now */
    }
    else    /* we're an existing menu */
    {
        /* make a happy string  */
        wsprintf(szText, "Memory %d @ %s", sNewID, lpszText);

        /* update our text */
        ModifyMenu(hmWindow, sNewID+ID_START, MF_BYCOMMAND|MF_STRING|MF_ENABLED,
                sNewID+ID_START, (LPSTR)szText);
    }

    FREEINFO;
#endif    
}