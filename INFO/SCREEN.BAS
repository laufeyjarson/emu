DECLARE FUNCTION CALCRAM! (h!, v!)
DECLARE FUNCTION MAKECOL! (mem!)
DECLARE FUNCTION CALCONE! (h!, v!)
DECLARE FUNCTION MAKEROW! (mem!)
PRINT "row: offs-end"
h = 1

FOR v = 1 TO 24
    PRINT v, CALCONE(h, v); MAKEROW(CALCONE(h, v)); MAKECOL(CALCONE(h, v));
    PRINT CALCONE(h + 39, v); MAKEROW(CALCONE(h + 39, v)); MAKECOL(CALCONE(h + 39, v))
NEXT v

END




FUNCTION CALCONE (h, v)
CALCONE = ((128 * v) + h - (984 * INT((v - 1) / 8)) + 895)
END FUNCTION

FUNCTION CALCRAM (h, v)
mem = ((128 * v) + h - (984 * INT((v - 1) / 8)) + 895)
CALCRAM = mem
END FUNCTION

FUNCTION MAKECOL (mem)

REM this uses most of MAKEROW.  MAKEROW contains the DOCS on what the
REM hell we're doing here...


StartOfScreen = 1024

offs = mem - StartOfScreen
chunk = (INT(mem / 128) - 8)


choffs = chunk * 128
part = offs - choffs
third = INT(part / 40)

col = part - (third * 40)
col = col + 1




MAKECOL = col

END FUNCTION

FUNCTION MAKEROW (mem)
REM the screen is stored as eight chunks, we call 0-7.
REM take the start of the screen (1024), and get an offs onto the
REM screen and then use the CHUNK of the screen we're on to decode
REM the LINE we're om

StartOfScreen = 1024

chunk = (INT(mem / 128) - 8)

offs = mem - StartOfScreen
choffs = chunk * 128

REM part is the offset into the chunk...
part = offs - choffs

REM third is the third of the screen we are on
third = INT(part / 40)

REM the row is the third*8 plus the row on that third...
row = (third * 8) + (chunk + 1)

REM the column is the offset into the part minus the start of the part
REM col = part - (third * 40)
REM col = col + 1

MAKEROW = row
END FUNCTION

