/****************************************************************************
	lgets.h
	prototype lgets
****************************************************************************/
char *lgets( PWSTR string, int n, HANDLE hFile);

UINT lputscr(WCHAR *szFoo, HANDLE hFile);
UINT lputs(WCHAR *szFoo, HANDLE hFile);
int lprintf(HANDLE hf, const WCHAR *fmt, ...);

