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

INT_PTR CALLBACK GetLoadAt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
short fileLoadRegs(HANDLE hFile, struct __reg *lpRegs);
short readDataLines(HANDLE hFile, char *lpNewMem, SFTYPE *lpFileInfo);
short fileGetAByte(WCHAR **parse, WCHAR *setVal);
long fileLoadReg(HANDLE hFile, short sVal, PCWSTR szBackup);
short seekString(HANDLE hFile, PWSTR szSearch, WCHAR *buff, short cbBuff);
void fileGetStr(short sVal, LPWSTR buf, short cbBuf, LPCWSTR backup);
void fileDoRegisters(HANDLE hFile);
void fileWriteDataBytes(HANDLE hFile, SFTYPE *lpFileInfo);
short cpuSaver(SFTYPE *lpFileInfo);
short cpuLoader(SFTYPE *lpFileInfo, char zero);
short cpuLoader(SFTYPE *lpFileInfo, char zero);
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
short cpuSaveFile(SFTYPE *lpFileInfo)
{
	short sRet;

	SpinCursor(SPIN_START);
	sRet = cpuSaver(lpFileInfo);
	SpinCursor(SPIN_STOP);
 	return sRet;
}

short cpuSaver(SFTYPE *lpFileInfo)
{
    WCHAR szBuf[128];
    DWORD bytesWritten;
    HANDLE hFile;

	SpinCursor(SPIN_SPIN);

    if(_waccess(lpFileInfo->szName, 0) == 0)
    {
        _wunlink(lpFileInfo->szName);
    }
    if((hFile = CreateFile(lpFileInfo->szName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        MessageBoxW(hWndMain, L"Cannot open file.", L"Error", MB_OK);
        return 1;
    }

	SpinCursor(SPIN_SPIN);

    switch(lpFileInfo->fileType)
    {
        case sttFile:
        fileGetStr(IDS_STTHDR, szBuf, 128, L"[Apple // Emulator STT System Status File]");
        lprintf(hFile, L"%s\r\n\r\n", szBuf);
        fileDoRegisters(hFile);
        fileWriteDataBytes(hFile, lpFileInfo);
        break;

        case memFile:
        fileGetStr(IDS_MEMHDR, szBuf, 128, L"[Apple // Emulator MEM Memory Image File]");
        lprintf(hFile, L"%s\r\n\r\n", szBuf);
        fileGetStr(IDS_START, szBuf, 128, L"Starting at:");
        lprintf(hFile, L"%s %04X\r\n", szBuf, lpFileInfo->usMemStart);
        fileGetStr(IDS_LENGTH, szBuf, 128, L"Length:");
        lprintf(hFile, L"%s %04X\r\n\r\n", szBuf, (lpFileInfo->usMemEnd-lpFileInfo->usMemStart));
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
                WriteFile(hFile, (LPCVOID) & ucVal, sizeof(unsigned char), &bytesWritten, NULL);
            }
        }
    }

    CloseHandle(hFile);
    return 0;
}


/*
 *  fileDoRegisters
 *
 *  write the register statuses
 */
void fileDoRegisters(HANDLE hFile)
{
    WCHAR szBuf[128];

	SpinCursor(SPIN_SPIN);

    fileGetStr(IDS_BEGINREG, szBuf, 128, L"Begin registers:");
    lprintf(hFile, L"%s\r\n", szBuf);

    if(r.pc != 0)
    {
        fileGetStr(IDS_PCREG,  szBuf, 128, L"Program Counter:");
        lprintf(hFile, L"%s %04X\r\n", szBuf, r.pc);
    }

	SpinCursor(SPIN_SPIN);

    if(r.a != 0)
    {
        fileGetStr(IDS_AREG,  szBuf, 128, L"Accumulator:");
        lprintf(hFile, L"%s %02X\r\n", szBuf, r.a);
    }

	SpinCursor(SPIN_SPIN);

    if(r.x != 0)
    {
        fileGetStr(IDS_XREG,  szBuf, 128, L"X-Register:");
        lprintf(hFile, L"%s %02X\r\n", szBuf, r.x);
    }

	SpinCursor(SPIN_SPIN);

    if(r.y != 0)
    {
        fileGetStr(IDS_YREG,  szBuf, 128, L"Y-Register:");
        lprintf(hFile, L"%s %02X\r\n", szBuf, r.y);
    }

	SpinCursor(SPIN_SPIN);

    if(r.s != 0)
    {
        fileGetStr(IDS_FLAGREG,  szBuf, 128, L"Flags:");
        lprintf(hFile, L"%s %02X\r\n", szBuf, r.s);
    }

	SpinCursor(SPIN_SPIN);

    if(r.sp != 0)
    {
        fileGetStr(IDS_SPREG,  szBuf, 128, L"Stack Pointer:");
        lprintf(hFile, L"%s %02X\r\n", szBuf, r.sp);
    }

	SpinCursor(SPIN_SPIN);

    if(r.halt != 0)
    {
        fileGetStr(IDS_HALTREG,  szBuf, 128, L"System Halted:");
        lprintf(hFile, L"%s %02X\r\n", szBuf, r.halt);
    }

	SpinCursor(SPIN_SPIN);

    if(r.ulTicks != 0L)
    {
        fileGetStr(IDS_TICKREG,  szBuf, 128, L"Tick Count:");
        lprintf(hFile, L"%s %08X\r\n", szBuf, r.ulTicks);
    }

	SpinCursor(SPIN_SPIN);

    fileGetStr(IDS_ENDREG, szBuf, 128, L"End registers.");
    lprintf(hFile, L"%s\r\n\r\n", szBuf);
}


/*
 *  fileWriteDataBytes
 *
 *  write data byte patterns
 */
void fileWriteDataBytes(HANDLE hFile, SFTYPE *lpFileInfo)
{
    long lCurr;
    short sCol;
    long lLine;
    WCHAR szBuf[129];
    unsigned char ucVal;

    fileGetStr(IDS_BEGINDATA, szBuf, 128, L"Begin data:");
    lprintf(hFile, L"%s\r\n", szBuf);

    sCol = 0;
    lLine = 0;
    lCurr = (long)lpFileInfo->usMemStart;

    while(lCurr <= (long)(lpFileInfo->usMemEnd))
    {
		SpinCursor(SPIN_SPIN);
        ucVal = (unsigned char) GetRam((unsigned short)lCurr);
        lprintf(hFile, L"%02X", ucVal);
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
                lprintf(hFile, L" ");
                break;
            case 32:
                lprintf(hFile, L"\r\n");
                sCol = 0;
                lLine++;
                if(lLine == 20)
                {
                    lLine = 0;
                    lprintf(hFile, L"\r\n");
                }
        }
    }

    if(sCol != 0)
        lprintf(hFile, L"\r\n");

    fileGetStr(IDS_ENDDATA, szBuf, 128, L"End data.");
    lprintf(hFile, L"%s\r\n\r\n", szBuf);
}


/*
 *  cpuLoadFile
 *
 *  read .stt or .mem file from disk.
 */ 
short cpuLoadFile(SFTYPE *lpFileInfo, char zero)
{
	short sRet;

	SpinCursor(SPIN_START);
	sRet = cpuLoader(lpFileInfo, zero);
	SpinCursor(SPIN_STOP);
	return sRet;
}

short cpuLoader(SFTYPE *lpFileInfo, char zero)
{
    struct __reg newRegs;
    HANDLE hNewMem;
    char *lpNewMem;
    HANDLE hFile;
    long lLoop;
    
    hNewMem = GlobalAlloc(GHND, (DWORD)0xFFFF);
    if(hNewMem == NULL)
    {
        MessageBoxW(hWndMain, L"Can't allocate load buffer.", L"Error", MB_OK);
        return 1;
    }
    lpNewMem = (char *)GlobalLock(hNewMem);

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

    if((hFile = CreateFile(lpFileInfo->szName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE )
    {
        MessageBoxW(hWndMain, L"Can't open file.", L"Error", MB_OK);
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
                if(MessageBoxW(hWndMain, L"No registers section was found, continue?", L"Error", MB_YESNO|MB_ICONQUESTION) == IDNO)
                {
                    GlobalUnlock(hNewMem);
                    GlobalFree(hNewMem);
                    CloseHandle(hFile);
                    return 1;
                }
            }
            /* fall thru to data read */
            
        case memFile:
            if(readDataLines(hFile, lpNewMem, lpFileInfo) == 0)
            {
                if(MessageBoxW(hWndMain, L"A read error occured; continue load?", L"Error",  MB_YESNO|MB_ICONQUESTION) == IDNO)
                {
                    GlobalUnlock(hNewMem);
                    GlobalFree(hNewMem);
                    CloseHandle(hFile);
                    return 1;
                }
            }
            break;

        default:    // read binary bytes into file!
		SpinCursor(SPIN_SPIN);
        ReadFile(hFile, (lpNewMem+lpFileInfo->usMemStart), ((lpFileInfo->usMemEnd - lpFileInfo->usMemStart)+1), NULL, NULL);
        break;
        
    }
    CloseHandle(hFile);

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
short readDataLines(HANDLE hFile, char *lpNewMem, SFTYPE *lpFileInfo)
{                                                         
    WCHAR szString[81];
    WCHAR szEnd[81];
    size_t cbEnd;
    long lLoc;
    long lStop;
    short sErr;
    WCHAR bVal;
    WCHAR *parse;

	SpinCursor(SPIN_SPIN);

    /* Get the "Begin data:" text */
    if(LoadStringW(hInst, IDS_BEGINDATA, szString, sizeof(szString)) == 0)
    {
        wsprintfW(szString, L"Begin data:");
    }
    if(seekString(hFile, szString, szEnd, sizeof(szEnd)) == FALSE)
    {
        return 0;
    }


	SpinCursor(SPIN_SPIN);

    /* Get the "End data." text */
    if(LoadString(hInst, IDS_ENDDATA, szEnd, sizeof(szString)) == 0)
    {
        wsprintf(szString, L"End data.");
    }

	SpinCursor(SPIN_SPIN);

    cbEnd = wcslen(szEnd);
    lLoc = (long)lpFileInfo->usMemStart;
    lStop = (long)lpFileInfo->usMemEnd + 1L;
    
    while(lgets(szString, sizeof(szString), hFile) != NULL)
    {
		SpinCursor(SPIN_SPIN);

        szString[wcslen(szString) - 1] = 0;
        if(wcsncmp(szString, szEnd, cbEnd) == 0)
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

            *(lpNewMem + lLoc) = (char)bVal;
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
    return 1;
}


/* 
 * fileGetAByte
 *
 * parse a byte from a string.
 * return 1 is ok byte, return 0 is end of string, return 2 is bad byte
 */
short fileGetAByte(WCHAR **parse, WCHAR *setVal)
{
    WCHAR *first, *last;
    long val = 0L;
    WCHAR szByte[3];
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
    val = wcstol(szByte, &last, 16);

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
short fileLoadRegs(HANDLE hFile, struct __reg *lpRegs)
{
    WCHAR szString[81];
    WCHAR buff[81];

    memset(lpRegs, 0, sizeof(struct __reg));

    /* Get the "Begin registers:" text */
    if(LoadString(hInst, IDS_BEGINREG, szString, sizeof(szString)) == 0)
    {
        wsprintf(szString, L"Begin registers:");
    }

	SpinCursor(SPIN_SPIN);

    /* verify "begin registers"*/ 
    if(seekString(hFile, szString, buff, sizeof(buff)) == FALSE)
    {
        return 0;
    }

	SpinCursor(SPIN_SPIN);
    lpRegs->pc = (unsigned short)fileLoadReg(hFile, IDS_PCREG, L"Program Counter:");
	SpinCursor(SPIN_SPIN);
    lpRegs->a  = (unsigned char) fileLoadReg(hFile, IDS_AREG,  L"Accumulator:");
	SpinCursor(SPIN_SPIN);
    lpRegs->x  = (unsigned char) fileLoadReg(hFile, IDS_XREG,  L"X-Register");
	SpinCursor(SPIN_SPIN);
    lpRegs->y  = (unsigned char) fileLoadReg(hFile, IDS_YREG,  L"Y-Register:");
	SpinCursor(SPIN_SPIN);
    lpRegs->s  = (unsigned char) fileLoadReg(hFile, IDS_FLAGREG,  L"Flags:");
	SpinCursor(SPIN_SPIN);
    lpRegs->sp = (unsigned char) fileLoadReg(hFile, IDS_SPREG,  L"Stack Pointer:");
	SpinCursor(SPIN_SPIN);
    lpRegs->halt=(unsigned char) fileLoadReg(hFile, IDS_HALTREG,  L"System Halted:");
	SpinCursor(SPIN_SPIN);
    lpRegs->ulTicks  = (unsigned long) fileLoadReg(hFile, IDS_TICKREG,  L"Tick Count:");
	SpinCursor(SPIN_SPIN);

    /* Get the "Begin registers:" text */
    if(LoadString(hInst, IDS_ENDREG, szString, sizeof(szString)) == 0)
    {
        wsprintf(szString, L"End registers.");
    }
    /* verify "end registers"*/ 
	SpinCursor(SPIN_SPIN);
    if(seekString(hFile, szString, buff, sizeof(buff)) == FALSE)
    {
        return 0;
    }
    return 1;
}


/* 
 *  fileGetStr
 *
 *  Load a string resource, w/backup
 */
void fileGetStr(short sVal, LPWSTR buf, short cbBuf, LPCWSTR backup)
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
long fileLoadReg(HANDLE hFile, short sVal, PCWSTR szBackup)
{
    WCHAR szString[81];
    WCHAR buff[80];
    WCHAR *first, *last;
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
        first = buff + wcslen(szString);
        while(*first != '\0' && isspace(*first))
            first++;
        /* now, read the hex digits */
        val = wcstol(first, &last, 16);
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
short seekString(HANDLE hFile, PWSTR szSearch, WCHAR *buff, short cbBuff)
{
    size_t cbSearch;
    WCHAR szBegin[81];

    /* Get the "Begin Data:" text */
    if(LoadString(hInst, IDS_BEGINDATA, szBegin, sizeof(szBegin)) == 0)
    {
        wsprintf(szBegin, L"Begin data:");
    }

    /* rewind the file */
    SetFilePointer(hFile, 0, NULL, 0);
    cbSearch = wcslen(szSearch);
    
    /* scan it for the string */
    while(lgets(buff, cbBuff, hFile) != NULL)
    {
        buff[wcslen(buff) -1] = '\0';
        if(wcsncmp(buff, szSearch, cbSearch) == 0)
        {
            return TRUE;
        }
        if(wcscmp(buff, szBegin) == 0)
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
short cpuBuildSFTYPE(SFTYPE *lpFileInfo, PWSTR szFileName)
{
    HANDLE hFile;
    WCHAR szRead[81], szSTThdr[81], szMEMhdr[81];
    WCHAR szStart[41], szLength[41];
    char fGotStart = 0, fGotLen = 0;
    size_t cbStart, cbLength;
    WCHAR *first, *last;
    long val;
    short retVal = 1;
    SFTYPE old;
    
    /* Get the file headers from the app or use the default */
    if(LoadString(hInst, IDS_STTHDR, szSTThdr, sizeof(szSTThdr)) == 0)
    {
        wsprintf(szSTThdr, L"[Apple // Emulator STT System Status File]");
    }
    if(LoadString(hInst, IDS_MEMHDR, szMEMhdr, sizeof(szMEMhdr)) == 0)
    {
        wsprintf(szSTThdr, L"[Apple // Emulator MEM Memory Image File]");
    }

    /* save old values */
    memcpy(&old, lpFileInfo, sizeof(SFTYPE));

    hFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return 1;
        
    /* read the first line from the file, and check it */
    if(lgets(szRead, 80, hFile) == NULL)
    {
        /* if lgets fails, blow*/
        CloseHandle(hFile);
        return 1;
    }
    szRead[wcslen(szRead) -1] = '\0';   // eat the \n
    
    /* This is a SYSTEM STATUS file - fill in the rest of the structure */
    if(_wcsicmp(szRead, szSTThdr) == 0)
    {
        wcscpy(lpFileInfo->szName, szFileName);
        lpFileInfo->fileType = sttFile;
        lpFileInfo->usMemStart = 0x0000;
        lpFileInfo->usMemEnd = 0xFFFF;
        dPrintf(L"identified an STT file\r\n");
        retVal = 0;
    }
    /* This is a MEMORY file - it's harder */
    else if(_wcsicmp(szRead, szMEMhdr) == 0)
    {
        wcscpy(lpFileInfo->szName, szFileName);
        lpFileInfo->fileType = memFile;
        lpFileInfo->usMemStart = 0x0000;
        lpFileInfo->usMemEnd = 0xFFFF;
        dPrintf(L"identified a MEM file\r\n");
        retVal = 0;
        
        /* Get the file headers from the app or use the default */
        if(LoadString(hInst, IDS_START, szStart, sizeof(szStart)) == 0)
        {
            wsprintf(szStart, L"Starting at:");
        }
        if(LoadString(hInst, IDS_LENGTH, szLength, sizeof(szLength)) == 0)
        {
            wsprintf(szLength, L"Length:");
        }

        cbStart = wcslen(szStart);
        cbLength = wcslen(szLength);
        /* try and load start and end values */
        while((fGotStart == 0 || fGotLen == 0) && lgets(szRead, 80, hFile) != NULL)
        {
            szRead[wcslen(szRead) -1] = '\0';   // eat the \n
            if(wcsncmp(szRead, szStart, cbStart) == 0)
            {
                /* we need to skip blanks */
                first = szRead + cbStart;
                while(*first != '\0' && isspace(*first))
                    first++;
                /* now, read the hex digits */
                val = wcstol(first, &last, 16);
                lpFileInfo->usMemStart = (unsigned short)val;   // truncation is ok
                fGotStart = 1;
            }
            else if(wcsncmp(szRead, szLength, cbLength) == 0)
            {
                /* we need to skip blanks */
                first = szRead + cbLength;
                while(*first != '\0' && isspace(*first))
                    first++;
                /* now, read the hex digits */
                val = wcstol(first, &last, 16);
                lpFileInfo->usMemEnd = (unsigned short)val; // truncation is ok
                fGotLen = 1;
            }
        }
        lpFileInfo->usMemEnd += lpFileInfo->usMemStart;
    }
    else    /* There is no header - this is a BINARY file of some sort 
               save some defaults */
    {
        wcscpy(lpFileInfo->szName, szFileName);
        lpFileInfo->fileType = binFile;
        dPrintf(L"Identified a BIN file\r\n");
        SetFilePointer(hFile, 0, NULL, 0);   // seek home
        val = SetFilePointer(hFile, 0, NULL, 2); // seek to end

        if(val == 0xFFFF)   // exactly 64K - likely a previous dump
        {
            lpFileInfo->usMemStart = 0;
            lpFileInfo->usMemEnd = 0xFFFF;
            retVal = 0;
        }
        else if(val > 0xFFFF)
        {
            if(MessageBox(NULL, L"This binary file is too long; truncate to 64K?", L"Error", MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
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
        else    // between 0 and 64K
        {
            if(DialogBoxParam(hInst, L"GETLOADAT", hWndMain, GetLoadAt, (LPARAM)(SFTYPE *)lpFileInfo) == TRUE)
            {
                // if this is too much, ask what to do.
                if((long)(lpFileInfo->usMemStart + val) > (0xFFFF))
                {
                    if(MessageBox(NULL, L"This is greater than 64K; truncate?", L"Error", MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
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


    CloseHandle(hFile);
    if(retVal == 1) // undo changes
        memcpy(lpFileInfo, &old, sizeof(SFTYPE));
    return retVal;
}


/*
 *  dlgproc for Get Load At dialog
 */
INT_PTR CALLBACK GetLoadAt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WCHAR *stop;
    long tempVal;
    WCHAR text[80];
    static SFTYPE *lpsft;

    switch (message)
    {
        case WM_INITDIALOG:            
            lpsft = (SFTYPE *)lParam;
            return (INT_PTR)TRUE;

        case WM_COMMAND:               
            if(wParam == IDCANCEL) 
            {
                EndDialog(hDlg, FALSE); 
                return (INT_PTR)TRUE;
            }
            if(wParam == IDOK)
            {
                GetDlgItemText(hDlg, IDC_STARTADD, text, 80);
                tempVal = wcstol(text, &stop, 16);
                if(tempVal > 0xffff)
                {
                    MessageBox(hDlg, L"Starting address too large", L"Error", MB_OK);
                    return TRUE;
                }
                lpsft->usMemStart = (unsigned short)tempVal;

                EndDialog(hDlg, TRUE);
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
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

#define PASSCOUNT 1500

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
			dPrintf(L"Loaded cursors...\r\n");
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
			dPrintf(L"\nstop spin\r\n");
			break;
	}

}
