/****************************************************************************

    PROGRAM: cpumem.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

	This contains data types and constants for the cpu's memory handling.
	    
****************************************************************************/

enum sfType {memFile, binFile, sttFile};

typedef struct __tagSFType {
	char szName[255];			// name of file to load/save
	enum sfType fileType;		// what we saved
	unsigned short usMemStart;	// starting location
	unsigned short usMemEnd;	// ending location
	char szReadableName[255];	// human readable file name
} SFTYPE;

#define SPIN_START 0
#define SPIN_SPIN  1
#define SPIN_STOP  2
#define MAX_CURS   8
