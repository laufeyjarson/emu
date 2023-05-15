/****************************************************************************

    PROGRAM: evideo.h

    PURPOSE: 6502 emulator engine

    FUNCTIONS:


    COMMENTS:

	This contains local data to the video driver only.  Noboy else uses this
	stuff.

****************************************************************************/


/* constants */
#define MAX_TEXT 1

/* display modes for the renderer */
#define TEXT_FULL	0
#define TEXT_MIXED	1
#define TEXT_NONE	2

struct stTextVideoTab
{
	unsigned short usStartAt;
	unsigned short usEndAt;
	unsigned short usXsize;
	unsigned short usYsize;	/* defaults and space for runtime */
	unsigned short usCursX;
	unsigned short usCursY;
	unsigned short usTextMode;
};

static struct stTextVideoTab stTextTab[MAX_TEXT+2] = {
	{0x00,	0,		0,	0, 	0, 0,	TEXT_FULL	},
	{0x400,	0x7ff,	40, 24, 0, 0, 	TEXT_FULL	},
	{0x00,	0,		0,	0,	0, 0,	TEXT_FULL	}
};

#define TEXT_WIDTH 8
#define TEXT_HEIGHT 12

#define GETVIDEOINFO lpVideo = GetWindowPointer(hWnd, 0)

#define MAX_LO_COLOR 16

static COLORREF crLoResColors[MAX_LO_COLOR] = {
	{RGB(0, 0, 0)},			//0 black
	{RGB(227, 108, 219)},	//1 pinkish
	{RGB(84, 252, 84)},		//2 chartruce
	{RGB(90, 245, 245)},	//3 lt blue
	{RGB(166, 169, 170)},	//4 gray
	{RGB(254, 245, 82)},	//5 yellow
	{RGB(147, 147, 117)},	//6 brown
	{RGB(80, 190, 73)},		//7 dk green
	{RGB(75, 51, 213)},		//8 blue
	{RGB(243, 75, 33)},		//9 red
	{RGB(177, 89, 187)},	//A purple!
	{RGB(249, 143, 28)},	//B orange
	{RGB(1, 27, 169)},		//C dk blue
	{RGB(67, 214, 158)},	//D teal 
	{RGB(149, 32, 32)},		//E dk red
	{RGB(255, 255, 255)}	//F white
};

	
