' Mikmod UniMod 5 ripper
' Programmed by Blade in Winter 2K
' Slow but does the job! ;)

DEFINT A-Z

DECLARE SUB copyUniModToFile (fromFile%, fromWhere&, howMuch&, toFile$)
DECLARE FUNCTION findUniModLen& (whichFile%)
DECLARE FUNCTION getUniModString$ (whichFile%)

TYPE uniModType
	numChn AS STRING * 1
	numPos AS INTEGER
	repPos AS INTEGER
	numPat AS INTEGER
	numTrk AS INTEGER
	numIns AS INTEGER
	initSpeed AS STRING * 1
	initTempo AS STRING * 1
	positions AS STRING * 256
	panning AS STRING * 32
	flags AS STRING * 1
END TYPE

DIM myWord AS INTEGER
DIM myLong AS LONG
DIM myId AS STRING * 4

ON ERROR GOTO errorHandler

myFile$ = COMMAND$
WHILE (myFile$ = "")
	LINE INPUT "File to rip: ", myFile$
WEND

OPEN myFile$ FOR INPUT AS 1
CLOSE 1
OPEN myFile$ FOR BINARY AS 1

FOR bc& = 1 TO LOF(1)
	GET 1, bc&, myId

	IF (myId = "UN05") THEN
		PRINT "Please wait, MikMod UniMod found, processing..."
		modLen& = findUniModLen(1)

		' Finally do the real stuff!
		copyUniModToFile 1, bc&, modLen&, "MIKMOD" + LTRIM$(STR$(fcount)) + ".UNI"
		fcount = fcount + 1
	END IF
NEXT

CLOSE 1
END

errorHandler:
PRINT "Error: "; ERROR$; "!"
END

SUB copyUniModToFile (fromFile, fromWhere&, howMuch&, toFile$)
	DIM myChar AS STRING * 1
	DIM id AS STRING * 4

	SEEK fromFile, fromWhere&
	GET fromFile, , id
	IF (id$ <> "UN05") THEN
		PRINT "Invalid Id: "; id
		EXIT SUB
	END IF
	SEEK fromFile, fromWhere&

	PRINT
	PRINT "Copying UniMod to "; toFile$; "...";

	aFile = FREEFILE
	OPEN toFile$ FOR BINARY AS aFile

	FOR i& = 1 TO howMuch&
		GET fromFile, , myChar
		PUT aFile, , myChar
	NEXT

	CLOSE aFile

	PRINT "done!"
END SUB

FUNCTION findUniModLen& (whichFile)
	DIM modHeader AS uniModType
	DIM myChar AS STRING * 1

	myModPos& = LOC(whichFile) - 4   ' we are 4 bytes away from the id

	GET whichFile, , modHeader

	PRINT
	PRINT "This UniMod sports:"
	PRINT ASC(modHeader.numChn); "channels"
	PRINT modHeader.numPat; "patterns"
	PRINT modHeader.numTrk; "tracks"
	PRINT modHeader.numIns; "instruments"

	PRINT
	PRINT "UniMod info:"
	PRINT "Name: "; getUniModString(whichFile)
	PRINT "Mod Type: "; getUniModString(whichFile)
	PRINT "Comment: "; getUniModString(whichFile)

	' Go through the instruments
	PRINT
	PRINT "Instruments:"
	FOR v = 1 TO modHeader.numIns
		GET whichFile, , myChar

		junk$ = SPACE$(208)
		GET whichFile, , junk$

		PRINT "Instrument"; v; "="; getUniModString(whichFile)

		FOR w = 1 TO ASC(myChar)    ' numsmp
			junk$ = SPACE$(5)
			GET whichFile, , junk$

			GET whichFile, , samLen&

			junk$ = SPACE$(8)
			GET whichFile, , junk$

			GET whichFile, , flags

			IF (flags AND 1) THEN samLen& = samLen& * 2
			totSamLen& = totSamLen& + samLen&

			PRINT " Sample"; w; "="; getUniModString(whichFile)
		NEXT
	NEXT

	' Go thru the patterns
	junk$ = SPACE$(modHeader.numPat * 2)
	GET whichFile, , junk$
	junk$ = SPACE$(modHeader.numPat * ASC(modHeader.numChn) * 2)
	GET whichFile, , junk$

	' Go thru the tracks
	FOR v = 1 TO modHeader.numTrk
		GET whichFile, , trkLen
		junk$ = SPACE$(trkLen)
		GET whichFile, , junk$
	NEXT

	' Finally go thru the samples
	FOR v& = 1 TO totSamLen&
		GET whichFile, , myChar
	NEXT

	' Now calculate module length
	myModPos& = LOC(whichFile) - myModPos&
	PRINT
	PRINT "UniMod length ="; myModPos&

	findUniModLen& = myModPos&
END FUNCTION

FUNCTION getUniModString$ (whichFile)
	GET whichFile, , myLen
	myStr$ = SPACE$(myLen)
	GET whichFile, , myStr$
	getUniModString$ = myStr$
END FUNCTION

