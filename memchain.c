/****************************************************************************

    PROGRAM: memchain.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    COMMENTS:

    This handles the meory chain.  The memory chain is a method of
    keeping all the windows and things in sync, and keeping things from
    eating the One True Memory Heap.
    
    The One True Memory Heap is a 64K chunk that lives here.  Things can
    ask for a pointer to it.  It will give them one.
****************************************************************************/

#include "emu.h" 

// These are the handle and long pointer to that handle for the 6502's
// 64K memory space.  These are kept LOCKED at all times, so the thing
// can run at a reasonable speed.  This may change to a situation that
// that locks them when the app has focus, being much less rude.
// This will likely change to an array of paragraphs, but that's later when
// I get the extended 80 col card in.
HANDLE hMemory;
BYTE * lpMemory;

#ifdef _DEBUG
unsigned short usMemCount = 0;
unsigned short usChainCount = 0;
#endif

/* This flag is to allow multiple writes before screen updates */
BOOL fCacheingUpdates = 0;

struct sMemChain
{
    struct sMemChain *pNext;
    unsigned short usStart;
    unsigned short usEnd;
    HWND hWnd;
};

struct sMemChain *psChain = NULL;

/* local prototypes */
void UpdateMemChain(unsigned short usStart, unsigned short usEnd, BOOL fForce);


/*
 *  Initalize the system's memory
 */
BOOL InitMemory(void)
{
    BYTE *lpInit;
    unsigned short i;
    
    hMemory = GlobalAlloc(GHND, (DWORD)(65536));
    if(hMemory == NULL)
        return 0;
    lpMemory = (char *)GlobalLock(hMemory);
    
    lpInit = lpMemory;
    for(i = 0; i < 0xffff; i++)
    {
        *lpInit = 0xEA;
        lpInit++;
    }    
    *lpInit = 0xff;
    return 1;
}
  
  
/*
 *  Initalize the registers
 */
BOOL InitRegisters(void)
{
    // zero all registers.
    memset(&r, 0, sizeof(struct __reg));

    // make sure the CPU is running when we turn it on
    r.halt = RUN;
    return 1;
}


/*
 *  Clean up the system memory
 */
BOOL FreeMemory(void)
{
    GlobalUnlock(hMemory);
    GlobalFree(hMemory);

#ifdef _DEBUG
    dPrintf(L"Memory cleared, %d locks.\r\n", usMemCount);
    dPrintf(L"chains left: %d\r\n", usChainCount);
#endif
    
    return 1;
}      


/*
 *  free register memory
 */
BOOL FreeRegisters(void)
{
    /* currently a no-op, but that may change */
    return 1;
}


#ifdef OLD
/*
 *  Get the value of a register
 */
unsigned short INLINE GetRegister(unsigned short eReg)
{
    switch(eReg)
    {
        case pc:
            return r.pc;
        case a:         
            return r.a;
        case x:
            return r.x;
        case y:
            return r.y;
        case s:
            return r.s;
        case sp:
            return r.sp;
        case halt:
            return r.halt;
        case ticks:
        default:
        return 0;
    }
}
#endif


/*
 *  Set a register to something
 */
unsigned short INLINE SetRegister(unsigned short eReg, unsigned short ucVal)
{
    switch(eReg)
    {
        case pc:
            r.pc = ucVal;
            return r.pc;
        case a:
            r.a = (unsigned char)ucVal;
            return r.a;
        case x:
            r.x = (unsigned char)ucVal;
            return r.x;
        case y:
            r.y = (unsigned char)ucVal;
            return r.y;
        case s:
            r.s = (unsigned char)ucVal;
            return r.s;
        case sp:
            r.sp = (unsigned char)ucVal;
            return r.sp;
        case halt:
            r.halt = (unsigned char)ucVal;
            return r.halt;
        case ticks:
        default:
        return 0;
    }
}


/*
 *  Get a byte from the system RAM and be happy
 */
unsigned char INLINE GetRam(unsigned short usAddr)
{
    return *(lpMemory+usAddr);
}

/*
 *  Set a value into the system memory
 */
unsigned char INLINE SetRam(unsigned short usAddr, BYTE ucVal)
{
    *(lpMemory+usAddr) = ucVal;
    UpdateMemChain(usAddr, usAddr, 0);
    return ucVal;
}

/*
 *  UpdateMemChain
 *
 *  Send an update message to all the windows watching a particular
 *  hunk of ram.
 *
 *  This can also cache messages if we're generating them inside the emulator
 *  and know there will be a mess of them (editing, or load/save)
 */
void UpdateMemChain(unsigned short usStart, unsigned short usEnd, BOOL fForce)
{
    struct sMemChain *pAt;

    /* This is to allow multiple-byte writes by the emulator... */
    static unsigned short usCacheStart = 0, usCacheEnd = 0;
	static BOOL fCacheInited = 0;
    
    /* if we're "forced" and not caching, don't send extras */
    if(fForce)
    {
        if(!fCacheingUpdates)
            return;
    }
    else if(fCacheingUpdates)   /* but, if we're not forced, and we are cacheing */
    {                           /* do, in fact, cache */
        /* store start and end values */
        if(usCacheStart > usStart)
            usCacheStart = usStart;
        if(usCacheEnd < usEnd)
            usCacheEnd = usEnd;
		if(fCacheInited == 0)
		{
			usCacheStart = usStart;
			usCacheEnd = usEnd;
			fCacheInited = 1;
		}
        return;
    }

    /* if we're cleaning up, set the cached values */
    if(fForce && fCacheingUpdates)
    {
        usStart = usCacheStart;
        usEnd   =  usCacheEnd;
    }

    /* If we need to send messages, do so now */
    for(pAt = psChain; pAt != NULL; pAt = pAt->pNext)
    {
        /* if it's in the range the window is watching . . .*/
		if(
			(usStart <= pAt->usStart && usEnd >= pAt->usEnd) ||	// we completely span the watch area
			(usStart >= pAt->usStart && usEnd <= pAt->usEnd) ||	// or we completely fit inside it
			(usStart <= pAt->usStart && (usEnd <= pAt->usEnd && usEnd >= pAt->usStart)) ||	// or we start before the block but end inside
			(usEnd >= pAt->usEnd && (usStart >= pAt->usStart && usStart <= pAt->usEnd))		// or we start inside but end outside
		  )
        {
            if(IsWindow(pAt->hWnd))
                SendMessage(pAt->hWnd, WM_MEMREFRESH, 0, MAKELONG(usStart, usEnd));
        }
    }

    /* if we were caching and we were forced, then clear cacheing */
    if(fForce && fCacheingUpdates)
    {
        usCacheStart = 0;
        usCacheEnd = 0;
		fCacheInited = 0;
    }
}

/*
 *  This startes the ram update cache thing so we don't get a flurry of messages.
 */
void StartUpdateCache(void)
{
    fCacheingUpdates = 1;
}

/*
 *  We're done updating, let us tell people...
 */
void StopUpdateCache(void)
{
    UpdateMemChain(0, 0, 1);
    fCacheingUpdates = 0;
}

/*
 *  JoinMemChain
 *
 *  Add yourself to the mem chain, usually at the end.
 */
short JoinMemChain(HWND hWnd, unsigned short usStart, unsigned short usEnd)
{
    struct sMemChain *pCurrent;
    struct sMemChain *pNew;
    
    /* Freshen an existing record, if one exists */
    for(pCurrent = psChain; pCurrent != NULL; pCurrent = pCurrent->pNext)
    {
        /* update this one and blow */
        if(pCurrent->hWnd == hWnd)
        {
            pCurrent->usStart = usStart;
            pCurrent->usEnd = usEnd;
            return 0;
        }        
    }
    
    /* gee, it's not there, must be a new one. */
    pNew = (struct sMemChain *)GlobalAllocPtr(GHND, sizeof(struct sMemChain));
    if(pNew == NULL)
    {
        dPrintf(L"Cannot allocate new chain!\r\n");
        return 1;
    }
#ifdef _DEBUG
        usChainCount++;
#endif                
    
    pNew->hWnd = hWnd;
    pNew->usStart = usStart;
    pNew->usEnd = usEnd;
    pNew->pNext = NULL;
    
    
    /* if this is a new LIST, allocate it, set it and blow */
    if(psChain == NULL)
    {
        psChain = pNew;
    }
    else
    {    
        /* ah, an existing list, and we need to add to the end */
        for(pCurrent = psChain; pCurrent->pNext != NULL; pCurrent = pCurrent->pNext)
        {
            /* do nothing here... */
        }
        pCurrent->pNext = pNew;
    }    
    return 0;
}
 
 
/*
 *  QuitMemChain
 *
 *  Takes a window out of the memory chain.
 */
short QuitMemChain(HWND hWnd)
{
    struct sMemChain *pHeld;
    struct sMemChain *pCurrent;
    
    /* if we're eating the head of the chain, do so, and quit */
    if(psChain->hWnd == hWnd)
    {
        pHeld = psChain->pNext;
        GlobalFreePtr(psChain);
#ifdef _DEBUG
        usChainCount--;
#endif                
        psChain = pHeld;
        return 0;
    }
    
    pHeld = NULL;
    /* great, we can keep the head of the list. */
    for(pCurrent = psChain; pCurrent != NULL; pCurrent = pCurrent->pNext)
    {
        /* eat me! */
        if(pCurrent->hWnd == hWnd)
        {
            if(pHeld == NULL)
            {
                dPrintf(L"can't delete null list!\r\n");
            }
            else
            {
                /* trim me from list (prev->next = my next */
                pHeld->pNext = pCurrent->pNext;
                GlobalFreePtr(pCurrent);
#ifdef _DEBUG
                usChainCount--;
#endif                
                return 0;
            }
        }
        /* save previous */
        pHeld = pCurrent;
    }
    return 1;
}

