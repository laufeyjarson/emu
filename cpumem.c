/****************************************************************************

    PROGRAM: cpumem.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

    This is code to handle creation, initaliztion, loading, saving, deleting,
    and updating of the emulated system's memory space.

    There will, eventually, be a standdard way of accessing memory; then any
    window in the queue looking at that hunk of the CPU's ram will get
    updated.  This will aid the Memory windows and the video display.

    There will also be a Load/Save function here, to load and save the
    system memory state.

    Please see FORMATS.TXT in the info directory for the file formats.

****************************************************************************/

#include "emu.h" 

BOOL EXPORT CALLBACK GetLoadAt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
short fileLoadRegs(HFILE hFile, struct __reg FAR *lpRegs);
short readDataLines(HFILE hFile, char *lpNewMem, SFTYPE FAR *lpFileInfo);
short fileGetAByte(char **parse, char *setVal);
long fileLoadReg(HFILE hFile, short sVal, char *szBackup);
short seekString(HFILE hFile, char *szSearch, char *buff, short cbBuff);
void fileGetStr(short sVal, char *buf, short cbBuf, char *backup);
void fileDoRegisters(HFILE hFile);
void fileWriteDataBytes(HFILE hFile, SFTYPE FAR *lpFileInfo);
short cpuSaver(SFTYPE FAR *lpFileInfo);
short cpuLoader(SFTYPE FAR *lpFileInfo, char zero);
short cpuLoader(SFTYPE FAR *lpFileInfo, char zero);
void SpinCursor(int iStat);



/*
 *  saveFile
 *
 *  Save a file.  Note that there are three currently supported file types:
 *  .MEM files -    these are a memory space, with start and end locations
 *                  stored right in the file.  A .MEM file cannot contain
 *                  register info.
 *  .BIN files -    these are a binary dump of a part of ram.  They contain
 *                  no reload information.
 *  .STT files -    this is a System sTaTus file.  It contains all of ram,
 *                  and the complete register table, including meta-registers.
 *                  This is a complete system save/restore.
 */
short cpuSaveFile(SFTYPE FAR *lpFileInfo)
{
	short sRet;

	SpinCursor(SPIN_START);
	sRet = cpuSaver(lpFileInfo);
	SpinCursor(SPIN_STOP);
 	return sRet;
}

short cpuSaver(SFTYPE FAR *lpFileInfo)
{
    char szBuf[128];
    HFILE hFile;

	SpinCursor(SPIN_SPIN);

    if(access(lpFileInfo->szName, 0) == 0)
    {
        unlink(lpFileInfo->szName);
    }
    if((hFile = _lcreat(lpFileInfo->szName, 0)) == HFILE_ERROR)
    {
        MessageBox(hWndMain, "Cannot open file.", "Error", MB_OK);
        return 1;
    }

	SpinCursor(SPIN_SPIN);

    switch(lpFileInfo->fileType)
    {
        case sttFile:
        fileGetStr(IDS_STTHDR, szBuf, 128, "[Apple // Emulator STT System Status File]");
        lprintf(hFile, "%s\r\n\r\n", szBuf);
        fileDoRegisters(hFile);
        fileWriteDataBytes(hFile, lpFileInfo);
        break;

        case memFile:
        fileGetStr(IDS_MEMHDR, szBuf, 128, "[Apple // Emulator MEM Memory Image File]");
        lprintf(hFile, "%s\r\n\r\n", szBuf);
        fileGetStr(IDS_START, szBuf, 128, "Starting at:");
        lprintf(hFile, "%s %04X\r\n", szBuf, lpFileInfo->usMemStart);
        fileGetStr(IDS_LENGTH, szBuf, 128, "Length:");
        lprintf(hFile, "%s %04X\r\n\r\n", szBuf, (lpFileInfo->usMemEnd-lpFileInfo->usMemStart));
        fileWriteDataBytes(hFile, lpFileInfo);
        break;

        case binFile:
        {
            long lLoop;
            unsigned char ucVal;
            
            for(lLoop = (long)lpFileInfo->usMemStart; lLoop <= (long)lpFileInfo->usMemEnd; lLoop++)
            {
				SpinCursor(SPIN_SPIN);

                ucVal = GetRam((unsigned short)lLoop);
                _lwrite(hFile, &ucVal, sizeof(unsigned char));
            }
        }
    }

    _lclose(hFile);
    return 0;
}


/*
 *  fileDoRegisters
 *
 *  write the register statuses
 */
void fileDoRegisters(HFILE hFile)
{
    char szBuf[128];

	SpinCursor(SPIN_SPIN);

    fileGetStr(IDS_BEGINREG, szBuf, 128, "Begin registers:");
    lprintf(hFile, "%s\r\n", szBuf);

    if(r.pc != 0)
    {
        fileGetStr(IDS_PCREG,  szBuf, 128, "Program Counter:");
        lprintf(hFile, "%s %04X\r\n", szBuf, r.pc);
    }

	SpinCursor(SPIN_SPIN);

    if(r.a != 0)
    {
        fileGetStr(IDS_AREG,  szBuf, 128, "Accumulator:");
        lprintf(hFile, "%s %02X\r\n", szBuf, r.a);
    }

	SpinCursor(SPIN_SPIN);

    if(r.x != 0)
    {
        fileGetStr(IDS_XREG,  szBuf, 128, "X-Register:");
        lprintf(hFile, "%s %02X\r\n", szBuf, r.x);
    }

	SpinCursor(SPIN_SPIN);

    if(r.y != 0)
    {
        fileGetStr(IDS_YREG,  szBuf, 128, "Y-Register:");
        lprintf(hFile, "%s %02X\r\n", szBuf, r.y);
    }

	SpinCursor(SPIN_SPIN);

    if(r.s != 0)
    {
        fileGetStr(IDS_FLAGREG,  szBuf, 128, "Flags:");
        lprintf(hFile, "%s %02X\r\n", szBuf, r.s);
    }

	SpinCursor(SPIN_SPIN);

    if(r.sp != 0)
    {
        fileGetStr(IDS_SPREG,  szBuf, 128, "Stack Pointer:");
        lprintf(hFile, "%s %02X\r\n", szBuf, r.sp);
    }

	SpinCursor(SPIN_SPIN);

    if(r.halt != 0)
    {
        fileGetStr(IDS_HALTREG,  szBuf, 128, "System Halted:");
        lprintf(hFile, "%s %02X\r\n", szBuf, r.halt);
    }

	SpinCursor(SPIN_SPIN);

    if(r.ulTicks != 0L)
    {
        fileGetStr(IDS_TICKREG,  szBuf, 128, "Tick Count:");
        lprintf(hFile, "%s %08X\r\n", szBuf, r.ulTicks);
    }

	SpinCursor(SPIN_SPIN);

    fileGetStr(IDS_ENDREG, szBuf, 128, "End registers.");
    lprintf(hFile, "%s\r\n\r\n", szBuf);
}


/*
 *  fileWriteDataBytes
 *
 *  write data byte patterns
 */
void fileWriteDataBytes(HFILE hFile, SFTYPE FAR *lpFileInfo)
{
    long lCurr;
    short sCol;
    long lLine;
    char szBuf[129];
    unsigned char ucVal;

    fileGetStr(IDS_BEGINDATA, szBuf, 128, "Begin data:");
    lprintf(hFile, "%s\r\n", szBuf);

    sCol = 0;
    lLine = 0;
    lCurr = (long)lpFileInfo->usMemStart;

    while(lCurr <= (long)(lpFileInfo->usMemEnd))
    {
		SpinCursor(SPIN_SPIN);
        ucVal = (unsigned char) GetRam((unsigned short)lCurr);
        lprintf(hFile, "%02X", ucVal);
        sCol++;
        lCurr++;
        switch(sCol)
        {
            case 4:
            case 8:
            case 12:
            case 16:
            case 20:
            case 24:
            case 28:
                lprintf(hFile, " ");
                break;
            case 32:
                lprintf(hFile, "\r\n");
                sCol = 0;
                lLine++;
                if(lLine == 20)
                {
                    lLine = 0;
                    lprintf(hFile, "\r\n");
                }
        }
    }

    if(sCol != 0)
        lprintf(hFile, "\r\n");

    fileGetStr(IDS_ENDDATA, szBuf, 128, "End data.");
    lprintf(hFile, "%s\r\n\r\n", szBuf);
}


/*
 *  cpuLoadFile
 *
 *  read .stt or .mem file from disk.
 */ 
short cpuLoadFile(SFTYPE FAR *lpFileInfo, char zero)
{
	short sRet;

	SpinCursor(SPIN_START);
	sRet = cpuLoader(lpFileInfo, zero);
	SpinCursor(SPIN_STOP);
	return sRet;
}

short cpuLoader(SFTYPE FAR *lpFileInfo, char zero)
{
    struct __reg newRegs;
    HANDLE hNewMem;
    char FAR *lpNewMem;
    HFILE hFile;
    long lLoop;
    
    hNewMem = GlobalAlloc(GHND, (DWORD)0xFFFF);
    if(hNewMem == NULL)
    {
        MessageBox(hWndMain, "Cann't allocate load buffer.", "Error", MB_OK);
        return 1;
    }
    lpNewMem = GlobalLock(hNewMem);

	SpinCursor(SPIN_SPIN);

    if(zero)
    {
        for(lLoop = 0; lLoop <= 0xFFFF; lLoop++)
        {
            *(lpNewMem + lLoop) = 0x00;
        }
    }
    else
    {
        for(lLoop = 0; lLoop <= 0xFFFF; lLoop++)
        {
            *(lpNewMem + lLoop) = GetRam((unsigned short)lLoop);
        }
    }
    
	SpinCursor(SPIN_SPIN);

    if((hFile = _lopen(lpFileInfo->szName, OF_READ|OF_SHARE_DENY_WRITE)) == HFILE_ERROR )
    {
        MessageBox(hWndMain, "Can't open file.", "Error", MB_OK);
        GlobalUnlock(hNewMem);
        GlobalFree(hNewMem);
        return 1;
    }

	SpinCursor(SPIN_SPIN);

    switch(lpFileInfo->fileType)
    {
        case sttFile:
            if(fileLoadRegs(hFile, &newRegs) == 0)
            {
                if(MessageBox(hWndMain, "No registers section was found, continue?", "Error", MB_YESNO|MB_ICONQUESTION) == IDNO)
                {
                    GlobalUnlock(hNewMem);
                    GlobalFree(hNewMem);
                    _lclose(hFile);
                    return 1;
                }
            }
            /* fall thru to data read */
            
        case memFile:
            if(readDataLines(hFile, lpNewMem, lpFileInfo) == 0)
            {
                if(MessageBox(hWndMain, "A read error occured; continue load?", "Error",  MB_YESNO|MB_ICONQUESTION) == IDNO)
                {
                    GlobalUnlock(hNewMem);
                    GlobalFree(hNewMem);
                    _lclose(hFile);
                    return 1;
                }
            }
            break;

        default:    // read binary bytes into file!
		SpinCursor(SPIN_SPIN);
        _lread(hFile, (lpNewMem+lpFileInfo->usMemStart), ((lpFileInfo->usMemEnd - lpFileInfo->usMemStart)+1));
        break;
        
    }
    _lclose(hFile);

	SpinCursor(SPIN_SPIN);

    /* if we're here, we are doing the load! */
	StartUpdateCache();
    for(lLoop = 0; lLoop < 0x10000; lLoop++)
    {
        SetRam((unsigned short)lLoop, *(lpNewMem+lLoop));
    }
	StopUpdateCache();
    memcpy(&r, &newRegs, sizeof(struct __reg));

	SpinCursor(SPIN_SPIN);

    GlobalUnlock(hNewMem);
    GlobalFree(hNewMem);

    return 0;
}


/*
 *  readDataLines
 *
 *  read in data lines from the file, checking for half-nybbles, and short loads.
 */
short readDataLines(HFILE hFile, char *lpNewMem, SFTYPE FAR *lpFileInfo)
{                                                         
    char szString[81];
    char szEnd[81];
    short cbEnd;
    long lLoc;
    long lStop;
    short sErr;
    char bVal;
    char *parse;

	SpinCursor(SPIN_SPIN);

    /* Get the "Begin data:" text */
    if(LoadString(hInst, IDS_BEGINDATA, szString, sizeof(szString)) == 0)
    {
        wsprintf(szString, "Begin data:");
    }
    if(seekString(hFile, szString, szEnd, sizeof(szEnd)) == FALSE)
    {
        return 0;
    }


	SpinCursor(SPIN_SPIN);

    /* Get the "End data." text */
    if(LoadString(hInst, IDS_ENDDATA, szEnd, sizeof(szString)) == 0)
    {
        wsprintf(szString, "End data.");
    }


	SpinCursor(SPIN_SPIN);

    cbEnd = strlen(szEnd);
    lLoc = (long)lpFileInfo->usMemStart;
    lStop = (long)lpFileInfo->usMemEnd + 1L;
    
    while(lgets(szString, sizeof(szString), hFile) != NULL)
    {
		SpinCursor(SPIN_SPIN);

        szString[strlen(szString) - 1] = '\0';
        if(strncmp(szString, szEnd, cbEnd) == 0)
        {
            if(lLoc != lStop)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
        parse = szString;
        while((sErr = fileGetAByte(&parse, &bVal)) == 1)
        {
			SpinCursor(SPIN_SPIN);

            *(lpNewMem + lLoc) = bVal;
            lLoc++;
            // if there's too much or an overflow, blow out
            if(lLoc > lStop)
            {
                return 0;
            }
        }

        if(sErr != 0)
        {
            return 0;
        }
    }
}


/* 
 * fileGetAByte
 *
 * parse a byte from a string.
 * return 1 is ok byte, return 0 is end of string, return 2 is bad byte
 */
short fileGetAByte(char **parse, char *setVal)
{
    char *first, *last;
    long val = 0L;
    char szByte[3];
    char retVal;

    /* we need to skip blanks */
    first = *parse;
    while(*first != '\0' && isspace(*first))
        first++;

    /* end of string */
    if(*first == '\0')
        return 0;

    /* check for two hex bytes */
    if(!isxdigit(*first) || !isxdigit(*(first + 1)) )
        return 2;   // bad byte

    /* copy to buffer */
    szByte[0] = *first;
    first++;
    szByte[1] = *first;
    first++;
    szByte[2] = '\0';

    /* now, read the hex digits */
    val = strtol(szByte, &last, 16);

    /* cvt to char, and point back */
    retVal = (char)val;
    *setVal = retVal;

    /* bump up parser*/
    *parse=first;
    return 1;
}

/*
 *  fileLoadRegs
 *
 *  This blits through the file repeatedly loading strings.  It'll be a little slower than
 *  it utterly needs to be, but it's robust, an it's simple to code.
 */
short fileLoadRegs(HFILE hFile, struct __reg FAR *lpRegs)
{
    char szString[81];
    char buff[81];

    memset(lpRegs, 0, sizeof(struct __reg));

    /* Get the "Begin registers:" text */
    if(LoadString(hInst, IDS_BEGINREG, szString, sizeof(szString)) == 0)
    {
        wsprintf(szString, "Begin registers:");
    }

	SpinCursor(SPIN_SPIN);

    /* verify "begin registers"*/ 
    if(seekString(hFile, szString, buff, sizeof(buff)) == FALSE)
    {
        return 0;
    }

	SpinCursor(SPIN_SPIN);
    lpRegs->pc = (unsigned short)fileLoadReg(hFile, IDS_PCREG, "Program Counter:");
	SpinCursor(SPIN_SPIN);
    lpRegs->a  = (unsigned char) fileLoadReg(hFile, IDS_AREG,  "Accumulator:");
	SpinCursor(SPIN_SPIN);
    lpRegs->x  = (unsigned char) fileLoadReg(hFile, IDS_XREG,  "X-Register");
	SpinCursor(SPIN_SPIN);
    lpRegs->y  = (unsigned char) fileLoadReg(hFile, IDS_YREG,  "Y-Register:");
	SpinCursor(SPIN_SPIN);
    lpRegs->s  = (unsigned char) fileLoadReg(hFile, IDS_FLAGREG,  "Flags:");
	SpinCursor(SPIN_SPIN);
    lpRegs->sp = (unsigned char) fileLoadReg(hFile, IDS_SPREG,  "Stack Pointer:");
	SpinCursor(SPIN_SPIN);
    lpRegs->halt=(unsigned char) fileLoadReg(hFile, IDS_HALTREG,  "System Halted:");
	SpinCursor(SPIN_SPIN);
    lpRegs->ulTicks  = (unsigned long) fileLoadReg(hFile, IDS_TICKREG,  "Tick Count:");
	SpinCursor(SPIN_SPIN);

    /* Get the "Begin registers:" text */
    if(LoadString(hInst, IDS_ENDREG, szString, sizeof(szString)) == 0)
    {
        wsprintf(szString, "End registers.");
    }
    /* verify "end registers"*/ 
	SpinCursor(SPIN_SPIN);
    if(seekString(hFile, szString, buff, sizeof(buff)) == FALSE)
    {
        return 0;
    }
}


/* 
 *  fileGetStr
 *
 *  Load a string resource, w/backup
 */
void fileGetStr(short sVal, char *buf, short cbBuf, char *backup)
{
    if(LoadString(hInst, sVal, buf, cbBuf) == 0)
    {
        wsprintf(buf, backup);
    }
}

/*
 *  fileLoadReg
 *
 *  This is a utility fn that loads a certin string from the file, and gets it's hex value.
 */
long fileLoadReg(HFILE hFile, short sVal, char *szBackup)
{
    char szString[81];
    char buff[80];
    char *first, *last;
    long val = 0L;

    /* Get the text */
    if(LoadString(hInst, sVal, szString, sizeof(szString)) == 0)
    {
        wsprintf(szString, szBackup);
    }
    /* load value */ 
    if(seekString(hFile, szString, buff, sizeof(buff)) == TRUE)
    {
        /* we need to skip blanks */
        first = buff + strlen(szString);
        while(*first != '\0' && isspace(*first))
            first++;
        /* now, read the hex digits */
        val = strtol(first, &last, 16);
    }
    return val;
}


/*
 *  seekString
 *
 *  This is a special fn - it will search the start of file, to
 *  the "Begin Data" line for a line starting with a string passed to
 *  it.
 *
 *  It returns TRUE if it finds it and buff will contain the complete
 *  line.  If FALSE, buff will be trash.
 */
short seekString(HFILE hFile, char *szSearch, char *buff, short cbBuff)
{
    short cbSearch;
    char szBegin[81];

    /* Get the "Begin Data:" text */
    if(LoadString(hInst, IDS_BEGINDATA, szBegin, sizeof(szBegin)) == 0)
    {
        wsprintf(szBegin, "Begin data:");
    }

    /* rewind the file */
    _llseek(hFile, 0, 0);
    cbSearch = strlen(szSearch);
    
    /* scan it for the string */
    while(lgets(buff, cbBuff, hFile) != NULL)
    {
        buff[strlen(buff) -1] = '\0';
        if(strncmp(buff, szSearch, cbSearch) == 0)
        {
            return TRUE;
        }
        if(strcmp(buff, szBegin) == 0)
        {
            return FALSE;
        }   
    }
    return FALSE;
}

/* 
 *  This will build an SFTYPE from only a filename, so the user
 *  can Open new files,
 *
 */
short cpuBuildSFTYPE(SFTYPE FAR *lpFileInfo, char *szFileName)
{
    HFILE hFile;
    char szRead[81], szSTThdr[81], szMEMhdr[81];
    char szStart[41], szLength[41];
    char fGotStart = 0, fGotLen = 0;
    unsigned int cbStart, cbLength;
    char *first, *last;
    long val;
    short retVal = 1;
    SFTYPE old;
    
    /* Get the file headers from the app or use the default */
    if(LoadString(hInst, IDS_STTHDR, szSTThdr, sizeof(szSTThdr)) == 0)
    {
        wsprintf(szSTThdr, "[Apple // Emulator STT System Status File]");
    }
    if(LoadString(hInst, IDS_MEMHDR, szMEMhdr, sizeof(szMEMhdr)) == 0)
    {
        wsprintf(szSTThdr, "[Apple // Emulator MEM Memory Image File]");
    }

    /* save old values */
    memcpy(&old, lpFileInfo, sizeof(SFTYPE));

    hFile = _lopen(szFileName, OF_READ|OF_SHARE_DENY_WRITE);
    if(hFile == HFILE_ERROR)
        return 1;
        
    /* read the first line from the file, and check it */
    if(lgets(szRead, 80, hFile) == NULL)
    {
        /* if lgets fails, blow*/
        _lclose(hFile);
        return 1;
    }
    szRead[strlen(szRead) -1] = '\0';   // eat the \n
    
    /* This is a SYSTEM STATUS file - fill in the rest of the structure */
    if(strcmpi(szRead, szSTThdr) == 0)
    {
        strcpy(lpFileInfo->szName, szFileName);
        lpFileInfo->fileType = sttFile;
        lpFileInfo->usMemStart = 0x0000;
        lpFileInfo->usMemEnd = 0xFFFF;
        dPrintf("identified an STT file\r\n");
        retVal = 0;
    }
    /* This is a MEMORY file - it's harder */
    else if(strcmpi(szRead, szMEMhdr) == 0)
    {
        strcpy(lpFileInfo->szName, szFileName);
        lpFileInfo->fileType = memFile;
        lpFileInfo->usMemStart = 0x0000;
        lpFileInfo->usMemEnd = 0xFFFF;
        dPrintf("identified a MEM file\r\n");
        retVal = 0;
        
        /* Get the file headers from the app or use the default */
        if(LoadString(hInst, IDS_START, szStart, sizeof(szStart)) == 0)
        {
            wsprintf(szStart, "Starting at:");
        }
        if(LoadString(hInst, IDS_LENGTH, szLength, sizeof(szLength)) == 0)
        {
            wsprintf(szLength, "Length:");
        }

        cbStart = strlen(szStart);
        cbLength = strlen(szLength);
        /* try and load start and end values */
        while((fGotStart == 0 || fGotLen == 0) && lgets(szRead, 80, hFile) != NULL)
        {
            szRead[strlen(szRead) -1] = '\0';   // eat the \n
            if(strncmp(szRead, szStart, cbStart) == 0)
            {
                /* we need to skip blanks */
                first = szRead + cbStart;
                while(*first != '\0' && isspace(*first))
                    first++;
                /* now, read the hex digits */
                val = strtol(first, &last, 16);
                lpFileInfo->usMemStart = (unsigned short)val;   // truncation is ok
                fGotStart = 1;
            }
            else if(strncmp(szRead, szLength, cbLength) == 0)
            {
                /* we need to skip blanks */
                first = szRead + cbLength;
                while(*first != '\0' && isspace(*first))
                    first++;
                /* now, read the hex digits */
                val = strtol(first, &last, 16);
                lpFileInfo->usMemEnd = (unsigned short)val; // truncation is ok
                fGotLen = 1;
            }
        }
        lpFileInfo->usMemEnd += lpFileInfo->usMemStart;
    }
    else    /* There is no header - this is a BINARY file of some sort 
               save some defaults */
    {
        strcpy(lpFileInfo->szName, szFileName);
        lpFileInfo->fileType = binFile;
        dPrintf("Identified a BIN file\r\n");
        _llseek(hFile, 0, 0);   // seek home
        val = _llseek(hFile, 0, 2); // seek to end

        if(val == 0xFFFF)   // exactly 64K - likely a previous dump
        {
            lpFileInfo->usMemStart = 0;
            lpFileInfo->usMemEnd = 0xFFFF;
            retVal = 0;
        }
        else if(val > 0xFFFF)
        {
            if(MessageBox(NULL, "This binary file is too long; truncate to 64K?", "Error", MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
            {
                lpFileInfo->usMemStart = 0;
                lpFileInfo->usMemEnd = 0xFFFF;
                retVal = 0;
            }
            else
            {
                retVal = 1;
            }
        }
        else    // between 0 anf 64K
        {
            if(DialogBoxParam(hInst, "GETLOADAT", hWndMain, GetLoadAt, (LPARAM)(SFTYPE FAR *)lpFileInfo) == TRUE)
            {
                // if this is too much, ask what to do.
                if((long)(lpFileInfo->usMemStart + val) > (0xFFFF))
                {
                    if(MessageBox(NULL, "This is greater than 64K; truncate?", "Error", MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
                    {
                        retVal = 0;
                        lpFileInfo->usMemEnd = 0xFFFF;
                    }
                    else    // don't truncate, blow
                    {
                        retVal = 1;
                    }
                }
                else    // don't need to truncate
                {
                    lpFileInfo->usMemEnd = (unsigned short)(lpFileInfo->usMemStart + val);
                    retVal = 0;
                }
            }
            else    // cancel load
            {
                retVal = 1;
            }
        }
    }


    _lclose(hFile);
    if(retVal == 1) // undo changes
        memcpy(lpFileInfo, &old, sizeof(SFTYPE));
    return retVal;
}


/*
 *  dlgproc for Get Load At dialog
 */
BOOL EXPORT CALLBACK GetLoadAt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char *stop;
    long tempVal;
    char text[80];
    static SFTYPE FAR *lpsft;

    switch (message)
    {
        case WM_INITDIALOG:            
            lpsft = (SFTYPE FAR *)lParam;
            return (TRUE);

        case WM_COMMAND:               
            if(wParam == IDCANCEL) 
            {
                EndDialog(hDlg, FALSE); 
                return (TRUE);
            }
            if(wParam == IDOK)
            {
                GetDlgItemText(hDlg, IDC_STARTADD, text, 80);
                tempVal = strtol(text, &stop, 16);
                if(tempVal > 0xffff)
                {
                    MessageBox(hDlg, "Starting address too large", "Error", MB_OK);
                    return TRUE;
                }
                lpsft->usMemStart = (unsigned short)tempVal;

                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;
    }
    return (FALSE);               
}

void SpinCursor(int iStat)
{
	static int iState = SPIN_STOP;
	int iLoader;
	static short sCount;
	static int iLoad[] = { IDC_SPIN1, IDC_SPIN2, IDC_SPIN3, IDC_SPIN4, 
						   IDC_SPIN5, IDC_SPIN6, IDC_SPIN7, IDC_SPIN8, 0};
	static HCURSOR ahLoaded[MAX_CURS];
	static unsigned long sulCount;


#ifdef _MAC
#define PASSCOUNT 750
#else
#define PASSCOUNT 1500
#endif
	switch(iStat)
	{
		case SPIN_START:
			if(iState == SPIN_STOP)
			{
				/* Load cursors */
				for (iLoader = 0; iLoader < MAX_CURS; iLoader++)
				{
					ahLoaded[iLoader] = LoadCursor(hInst, MAKEINTRESOURCE(iLoad[iLoader]));
				}
			}
			dPrintf("Loaded cursors...\r\n");
			sulCount=MAX_CURS-1;
			iState = SPIN_SPIN;
			sCount = 0;
			/* fall thru */

		case SPIN_SPIN:
			sCount++;
			if(sCount > PASSCOUNT)
			{
				SetCursor(ahLoaded[sulCount%MAX_CURS]);
				sulCount++;
				sCount = 0;
			}
			break;

		case SPIN_STOP:
			SetCursor(LoadCursor(hInst, IDC_ARROW));
 			if(iState == SPIN_SPIN)
			{
				/* Unload cursors */
#if 0
				for (iLoader = 0; iLoader < MAX_CURS; iLoader++)
				{
					DeleteObject((HGDIOBJ)ahLoaded[iLoader]);
				}
#endif
			}
			iState = SPIN_STOP;
			dPrintf("\nstop spin\r\n");
			break;
	}

}
