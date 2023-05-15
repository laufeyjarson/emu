/****************************************************************************

    PROGRAM: emufile.c

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

	This is the file menu functions.  Mostly load/save stuff.
	    
****************************************************************************/

#include "emu.h"

static char fIsFileOpen = 0;
static SFTYPE fileInfo;
static char fClearMemory = 0;

void fileUpdateTitle(void);
void fileBuildOFN(HWND hWnd, char fSave, OPENFILENAME FAR *lpofn);
int fileGetStartEnd(HWND hWnd);
UINT_PTR EXPORT CALLBACK fileGetSEDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


/*
 *	FileSaveMenu - save the file they opened
 */
void FileSaveMenu(HWND hWnd)
{
	if(fIsFileOpen == 0)		// If there is no file open, then do a save as
	{
		FileSaveAsMenu(hWnd);
	}
	/* let other code actually save */
	cpuSaveFile((SFTYPE FAR *)&fileInfo);
}

/*
 *	Let them save a file as a certian type.  The type set in the box will
 *	be used to save the file, regardless of name or extension.
 */
void FileSaveAsMenu(HWND hWnd)
{
	OPENFILENAME ofn;
	SFTYPE old;
#ifdef _DEBUG
	DWORD dwError;
#endif	

	/* keep a copy of the SFTYPE we have now */
	memcpy(&old, &fileInfo, sizeof(SFTYPE));
	
	fileBuildOFN(hWnd, 1, (OPENFILENAME FAR *)&ofn);
	if (GetOpenFileName(&ofn)) 
	{
		wcscpy(fileInfo.szName, ofn.lpstrFile);

		/* set file type, get start/end info if needed */
		switch(ofn.nFilterIndex)
		{
			case 1:
			fileInfo.fileType = sttFile;
			fileInfo.usMemStart = 0000;
			fileInfo.usMemEnd = 0xFFFF;
			break;
			
			case 2:
			fileInfo.fileType = memFile;
			if(fileGetStartEnd(hWnd) == FALSE)
			{
				/* undo work */
				memcpy(&fileInfo, &old, sizeof(SFTYPE));
				fileUpdateTitle();
				return;
			}
			break;
			
			case 3:
			default:
			fileInfo.fileType = binFile;
			if(fileGetStartEnd(hWnd) == FALSE)
			{
				/* undo work */
				memcpy(&fileInfo, &old, sizeof(SFTYPE));
				fileUpdateTitle();
				return;
			}
			break;
		}
		/* the fileinfo block is updated, and we are OK - show a file */
		fIsFileOpen = 1;
		fileUpdateTitle();

		/* let other code actually save */
		cpuSaveFile((SFTYPE FAR *)&fileInfo);

	}
	else
	{
#ifdef _DEBUG
		dwError = CommDlgExtendedError();
		dPrintf(L"A commdlg error %ld occurred\r\n", dwError);
#endif		
		fileUpdateTitle();
	}
}

/*
 *	Handle getting things dropped on us.
 *
 */

#ifdef WIN32
void FileOpenDroppedFile(HWND hWnd, HDROP hDrop)
{
	WCHAR szFileName[256];

	if(fIsFileOpen == 1)
	{
		FileCloseMenu(hWnd);		// gotta close an open file
		if(fIsFileOpen == 1)		// cancelled close
			return;
	}
	
	DragQueryFile(hDrop, 0, szFileName, 255);
	DragFinish(hDrop);

	GetFileTitle(szFileName, fileInfo.szReadableName, sizeof(fileInfo.szReadableName)-1);

	if(cpuBuildSFTYPE((SFTYPE FAR *)&fileInfo, szFileName) == 0)
	{
		if(cpuLoadFile((SFTYPE FAR *)&fileInfo, 1) == 0)
		{
			fIsFileOpen = 1;
			fileUpdateTitle();
		}
	}
	fileUpdateTitle();
}
#endif


/*
 *	Open a saved file.
 */
void FileOpenMenu(HWND hWnd)
{
	OPENFILENAME ofn;
#ifdef _DEBUG
	DWORD dwError;
#endif	
	if(fIsFileOpen == 1)
	{
		FileCloseMenu(hWnd);		// gotta close an open file
		if(fIsFileOpen == 1)		// cancelled close
			return;
	}
	
	fileBuildOFN(hWnd, 0, (OPENFILENAME FAR *)&ofn);
	if (GetOpenFileName(&ofn)) 
	{
		if(cpuBuildSFTYPE((SFTYPE FAR *)&fileInfo, ofn.lpstrFile) == 0)
		{
			if(cpuLoadFile((SFTYPE FAR *)&fileInfo, fClearMemory) == 0)
			{
				fIsFileOpen = 1;
				fileUpdateTitle();
			}
		}
	}
	else
	{
#ifdef _DEBUG
		dwError = CommDlgExtendedError();
		dPrintf(L"A commdlg error %ld occurred\r\n", dwError);
#endif		
		fileUpdateTitle();
	}
}


/*
 *	Close the current file
 */
void FileCloseMenu(HWND hWnd)
{
	int iRet;
	
	if(fIsFileOpen == 0)
		return;
	
	iRet = MessageBox(hWnd, L"Save changes to file?", L"Close File", MB_YESNOCANCEL|MB_ICONQUESTION);
	switch(iRet)
	{
		case IDYES:
		FileSaveMenu(hWnd);
		break;
		
		case IDNO:
		break;
		
		case IDCANCEL:
		return;
	}
	fIsFileOpen = 0;
	fileUpdateTitle();
}


/*
 *	Create a new file
 */
void FileNewMenu(HWND hWnd)
{
	if(fIsFileOpen == 1)
	{
		FileCloseMenu(hWnd);		// gotta close an open file
		if(fIsFileOpen == 1)		// cancelled close
			return;
	}

	/* Create a brand new .STT file - they are simplest to fake! */
	fIsFileOpen = 1;
	wcscpy(fileInfo.szName, L"UNTITLED.STT");
	wcscpy(fileInfo.szReadableName, L"Untitled");
	fileInfo.fileType = sttFile;
	fileInfo.usMemStart = 0;
	fileInfo.usMemEnd = 0xFFFF;
	fileUpdateTitle();
} 
 

/*
 *	Update the main window's title, to reflect the file loaded, if there
 *	is, in fact, a file loaded.
 */
void fileUpdateTitle(void)
{
	WCHAR szWindowTitle[128];
	WCHAR szFile[_MAX_FNAME], szExt[_MAX_EXT];
	
	if(LoadString(hInst, IDS_APPWINDOW, szWindowTitle, sizeof(szWindowTitle)) == 0)
	{
		wsprintf(szWindowTitle, L"6502 Emulator");
	}

	if(fIsFileOpen == 1)
	{
		wcscat(szWindowTitle, L" - ");

		if(wcslen(fileInfo.szReadableName) == 0)
		{
			_wsplitpath(fileInfo.szName, NULL, NULL, szFile, szExt);
			wcscat(szWindowTitle, szFile);
			wcscat(szWindowTitle, szExt);
		}
		else
		{
			wcscat(szWindowTitle, fileInfo.szReadableName);
		}
	}
	SetWindowText(hWndMain, szWindowTitle);
}

/*
 *	fileBuildOFN
 *
 *	Builds most of an OPENFILENAME record for commdlg.  Saves code.
 */
void fileBuildOFN(HWND hWnd, char fSave, OPENFILENAME FAR *lpofn)
{
	static WCHAR szDir[256];
	static WCHAR szFile[256];
	static WCHAR szFileTitle[256];
	size_t cbString;
	unsigned int ui;
	WCHAR chReplace;
	static WCHAR szFilter[256];
	
	/* fill things we can get easily */
	_wgetcwd(szDir, sizeof(szDir));	// get current directory
	szFile[0] = '\0';				// no file
	szFileTitle[0] = '\0';
	
	/* load the filter string from the string table */
	if((cbString = LoadString(hInst, IDS_MEMFLAGS, szFilter,
		sizeof(szFilter))) == 0)
	{
		/* If the string table breaks, use default */
		wsprintf(szFilter, L"All Files(*.*)|*.*||");
		cbString = wcslen(szFilter);
	}

	/* get null placeholder and replace those with nulls */
	chReplace = szFilter[cbString -1];
	for(ui = 0; szFilter[ui] != '\0'; ui++)
	{
		if(szFilter[ui] == chReplace)
			szFilter[ui] = '\0';
	}

	/* clear to zero, and fill struct values */
	memset(lpofn, 0, sizeof(OPENFILENAME));
	lpofn->lStructSize = sizeof(OPENFILENAME);
	lpofn->hwndOwner = hWnd;
	lpofn->lpstrFilter = szFilter;
	lpofn->nFilterIndex = 1;
	lpofn->lpstrFile= szFile;
	lpofn->nMaxFile = sizeof(szFile);
	lpofn->lpstrFileTitle = szFileTitle;
	lpofn->nMaxFileTitle = sizeof(szFileTitle);
	lpofn->lpstrInitialDir = szDir;
	lpofn->hInstance = hInst;
	if(fSave)
	{
		lpofn->Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
		lpofn->lpstrTitle = L"Save As...";
	}
	else	/* open */
	{
		lpofn->lpfnHook = FileOpenHookProc;
		lpofn->lpTemplateName = MAKEINTRESOURCE(IDD_OPENDLG);
		fClearMemory = 0;
		lpofn->Flags = OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
						OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
		lpofn->lpstrTitle = L"Open...";
	}
	lpofn->lpstrFileTitle = fileInfo.szReadableName;
	lpofn->nMaxFileTitle = sizeof(fileInfo.szReadableName)-1;
}


/*
 *	This is the hook fn for the Open dialog to set the Zero Ram flag.
 */
UINT_PTR EXPORT CALLBACK FileOpenHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static OPENFILENAME FAR *lpofn;
    switch(msg) 
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            /* Use IsDlgButtonChecked to set lCustData. */
            if (wParam == IDOK) 
            {
                /* Set zero flag. */
                fClearMemory = (unsigned char)IsDlgButtonChecked(hdlg, IDC_ZEROMEM);
            }
            return FALSE; /* Allow standard processing. */
    }

    /* Allow standard processing. */
    return FALSE;
}


/*
 *	fileGetStartEnd
 *
 */
int fileGetStartEnd(HWND hWnd)
{
	return (int)DialogBox(hInst, L"GETSTARTEND", hWnd, fileGetSEDlg);
}

UINT_PTR EXPORT CALLBACK fileGetSEDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WCHAR *stop;
	long tempVal;
	WCHAR text[80];

    switch (message)
    {
        case WM_INITDIALOG:            
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
				tempVal = wcstol(text, &stop, 16);
				if(tempVal > 0xffff)
				{
					MessageBox(hDlg, L"Starting address too large", L"Error", MB_OK);
					return TRUE;
				}
				fileInfo.usMemStart = (unsigned short)tempVal;

            	GetDlgItemText(hDlg, IDC_ENDADD, text, 80);
				tempVal = wcstol(text, &stop, 16);
				if(tempVal > 0xffff)
				{
					MessageBox(hDlg, L"Ending address too large", L"Error", MB_OK);
					return TRUE;
				}
				fileInfo.usMemEnd = (unsigned short)tempVal;

				/* swap start and end if reversed */
				if(fileInfo.usMemEnd < fileInfo.usMemStart)
				{
					tempVal = (long)fileInfo.usMemEnd;
					fileInfo.usMemEnd = fileInfo.usMemStart;
					fileInfo.usMemStart = (unsigned short)tempVal;
				}

            	EndDialog(hDlg, TRUE);
            	return TRUE;
            }
            break;
    }
    return (FALSE);               
}

