/****************************************************************************

    PROGRAM: einstr.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:

    COMMENTS:

****************************************************************************/

/*
 *  I used to have two functions, now I have one and some macros.
 *
 *  This means that a bug in my memory addressing has to be fixed only once.
 */
#define GET 0
#define SET 1
#define GetVal(o)       GetSetVal((o), GET, 0)
#define SetVal(o, v)    GetSetVal((o), SET, (v))
