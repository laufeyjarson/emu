void try1(void);
void try2(void);
void main(void);
void CalcXYFromScreen1(unsigned short *usRow, unsigned short *usCol, unsigned short usMem);
void CalcXYFromScreen2(unsigned short *usRow, unsigned short *usCol, unsigned short usMem);

void main(void)
{
	try1();
	try2();
}

#define RUNS 1000000L

void try1(void)
{
	unsigned long x;
	unsigned short ucX, ucY;

	for(x = 0; x < RUNS; x++)
	{
		CalcXYFromScreen1(&ucX, &ucY, 1025);
	}
}

void try2(void)
{
	unsigned long x;
	unsigned short ucX, ucY;

	for(x = 0; x < RUNS; x++)
	{
		CalcXYFromScreen2(&ucX, &ucY, 1025);
	}
}


#define StartOfScreen 1024

/*
 *	Make sure it's ok for us to fill in usRow and usCol...
 */
void CalcXYFromScreen1(unsigned short *usRow, unsigned short *usCol, unsigned short usMem)
{
	register unsigned short usChunk;
	register unsigned short usOffs;
	register unsigned short usChOffs;
	register unsigned short usPart;
	register unsigned short usThird;

	/* The Chunk of the screen is the interleave id, 0-7 */
	usChunk = ( ((unsigned short)(usMem / 128)) - 8);

	/* The Offs is the offset from the start of the scree */
	usOffs = usMem - StartOfScreen;

	/* The ChOffs is the offset in ram of the start of the chunk. */
	usChOffs = usChunk * 128;

	/* Part is the offset into the chunk... */
	usPart = usOffs - usChOffs;

	/* Third is the third of the screen we are on */
	usThird = (unsigned short)(usPart / 40);

	/* The row is the third times 8 (8 rows per third) plus the row on that third... */
	*usRow = (usThird * 8) + (usChunk + 1);

	/* The column is the offset into the part minus the start of the part */
	*usCol = (usPart - (usThird * 40)) +1;
}

void CalcXYFromScreen2(unsigned short *usRow, unsigned short *usCol, unsigned short usMem)
{
	/* This is the same as above, but crammed into two giant expressions that I will let
	   the compiler optimize.  We'll run 10000 of them and see how long each one takes.
	   The profiler is our pal...   */

	*usRow = (((unsigned short)((usMem - StartOfScreen - 
			(( ((unsigned short)(usMem / 128)) - 8) * 128)) / 40)) * 8) + 
			(( ((unsigned short)(usMem / 128)) - 8) + 1);

	/* The column is the offset into the part minus the start of the part */
	*usCol = ((usMem - StartOfScreen - (( ((unsigned short)(usMem / 128)) - 8) * 128)) - 
			(((unsigned short)((usMem - StartOfScreen -
			(( ((unsigned short)(usMem / 128)) - 8) * 128)) / 40)) * 40)) +1;
}

