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
	
	Return Value
	
	If successful, the lgets function returns string. It returns NULL to
	indicate either an error or end-of-file condition.
****************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

char *lgets( char *string, unsigned int n, HFILE hFile)
{
	long curPos;
	unsigned int ui;
	
	curPos = _llseek(hFile, 0, 1);	// get the current posisiton
	if(_lread(hFile, string, n-1) == HFILE_ERROR)
	{								// read a hunk of string
		return NULL;
	}

	string[n] = '\0';				// null terminate string

	for(ui = 0; string[ui] != '\0' && string[ui] != '\n'; ui++)
		;							// count bytes that we are keeping

	if(string[ui] == '\n')			// keep the newline
	{
		if(string[ui-1] == '\r')
		{
			string[ui-1] = '\n';
		}
		else if(ui+1 < n)
		{
			ui++;
		}
	}

	string[ui] = '\0';				// null terminate at \r

	_llseek(hFile, curPos + ui +1, 0);	// seek to the next byte
	return string;
}

/*
 fputs w/cr 
 */
UINT lputscr(char *szFoo, HFILE hFile)
{
	UINT uiErr, uiErr2;
	uiErr = _lwrite(hFile, szFoo, strlen(szFoo));
	if(uiErr == HFILE_ERROR)
		return uiErr;
	uiErr2 = _lwrite(hFile, "\r\n", 2);
	if(uiErr2 == HFILE_ERROR)
		return uiErr2;

	return uiErr+uiErr2;
}

/* fputs clone */
UINT lputs(char *szFoo, HFILE hFile)
{
	return _lwrite(hFile, szFoo, strlen(szFoo));
}


/*
 *	lprintf - fpritf knock off
 */
int lprintf(HFILE hf, char *fmt, ...)
{
	va_list vlist;
	static char pBuff[4096];
	int rtn;
	
	// do the printf
	va_start(vlist, fmt);
	rtn = wvsprintf(pBuff, fmt, vlist);
	va_end(vlist);
	
	_lwrite(hf, pBuff, rtn);
	return rtn;
}
