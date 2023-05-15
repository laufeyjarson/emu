/****************************************************************************

    PROGRAM: listman.h

    PURPOSE: manage a list of things that we can search

    FUNCTIONS:


    COMMENTS:

    These are data types for a list of things, and lists of things.
    
****************************************************************************/

#ifndef __LISTMAN_H_
#define __LISTMAN_H_

/*
 *  This is the BLOB type that we manipulate.  Notice the complex
 *  definition - not my usual style, but we're using a type stolen
 *  from NT and OLE, so I wanted to avoid conflicts, but also not include
 *  all those ugly headers, if we don't need them.
 */
#ifndef _tagBLOB_DEFINED    /* if there's no blob defined... */

#define _tagBLOB_DEFINED
#define _BLOB_DEFINED
#define _LPBLOB_DEFINED

typedef struct  tagBLOB
{
    unsigned long cbSize;
    char *pBlobData;
}   BLOB;

typedef struct tagBLOB FAR *LPBLOB;

#endif


/*
 *  This is the list type that we declare one of.
 */
 
typedef struct tagLIST
{
    struct tagLIST *pList;
}   LIST;

#endif /* __LISTMAN_H_ */