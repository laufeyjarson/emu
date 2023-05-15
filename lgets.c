/****************************************************************************

	FILE: lgets.c
	
	Syntax 	char *lgets( char *string, int n, HFILE hFile );

	Parameter		Description
	string			Storage location for data
	n				Maximum number of characters to read
	hFile			Handle to file to read

	The lgets function reads a string from the hFile file argument and stores
	it in string. Characters are read from the current position up to and
	including the first newline character ('\n'), up to the end of the file,
	or until the number of characters read is equal to n-1, whichever comes
	first. The result is stored in string, and a null character ('\0') is
	appended. The newline character, if read, is included in the string.

	Note that this program expects to read and write pure ASCII files.
	As the contents are constrained to something that should never contain
	extended characters, this should not be a problem.  The program will
	refuse to read a file with a BOM.

	This is mostly because I'm too lazy to handle all the translations
	from the different BOM to the internal type.  All the files remain
	ASCII.
	
	Return Value
	
	If successful, the lgets function returns string. It returns NULL to
	indicate either an error or end-of-file condition.
****************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strsafe.h>

static WCHAR _wcBuff[4096];
static char _cBuff[4096];

LPWSTR lgets( LPWSTR string, unsigned int n, HANDLE hFile )
{
	long curPos;
	unsigned int ui;

	if (n > sizeof(_cBuff))
		n = (unsigned int)sizeof(_cBuff);
	
	curPos = SetFilePointer(hFile, 0, NULL, 1);	// get the current posisiton
	if(!ReadFile(hFile, (LPVOID)_cBuff, n-1, NULL, NULL))
	{								// read a hunk of string
		return NULL;
	}

	_cBuff[n] = '\0';				// null terminate _cBuff

	for(ui = 0; _cBuff[ui] != '\0' && _cBuff[ui] != '\n'; ui++)
		;							// count bytes that we are keeping

	if(_cBuff[ui] == '\n')			// keep the newline
	{
		if(_cBuff[ui-1] == '\r')
		{
			_cBuff[ui-1] = '\n';
		}
		else if(ui+1 < n)
		{
			ui++;
		}
	}

	_cBuff[ui] = '\0';				// null terminate at \r
	SetFilePointer(hFile, curPos + ui + 1, NULL, 0);	// seek to the next byte

	MultiByteToWideChar(CP_ACP, 0, _cBuff, ui+1, string, n);
	return string;
}

/*
 fputs w/cr 
 */
UINT lputscr(WCHAR *szFoo, HANDLE hFile)
{
	lprintf(hFile, "%s\r\n");
	return 0;
}

/* fputs clone */
UINT lputs(WCHAR *szFoo, HANDLE hFile)
{
	lprintf(hFile, "%s", szFoo);
	return 0;
}

/*
 *	lprintf - fprintf knock off
 */
int lprintf(HANDLE hf, const WCHAR *fmt, ...)
{
	va_list vlist;
	int rtn;
	
	// do the printf
	va_start(vlist, fmt);
	StringCchVPrintfW(_wcBuff, sizeof(_wcBuff), fmt, vlist);
	va_end(vlist);
	
	WideCharToMultiByte(CP_ACP, 0, _wcBuff, -1, _cBuff, sizeof(_cBuff), NULL, NULL);
	rtn = (int)strlen(_cBuff);
	WriteFile(hf, _cBuff, rtn, NULL, NULL);
	return rtn;
}
