'****************************************************************************'
'                                                                            '
' Text file condensing utility. Version 4.0                                  '
'                                                                            '
' Copyright (c) Samuel Gomes (Blade), 1998-2011.                             '
' All rights reserved.                                                       '
'                                                                            '
'*************************************************************************** '

#Include "file.bi"

Const FALSE = 0
Const TRUE = Not FALSE

' Generates a temporary filename
Function TempFile() As String
	Return Environ("TEMP") & "\" & Fix(Timer) & ".tmp"
End Function

' Check if an atgument is present in the command line
Function ArgVPresent(argv As String) As Integer
	Dim argc As Integer = 1
	Dim As String a, b
	
	b = "/" + argv
	Do
		a = Command(argc)
		If (Len(a) = 0) Then Exit Do
		
		If UCase(a) = UCase(b) Then Return TRUE
		
		argc += 1
	Loop
	
	Return FALSE
End Function

' Performs extra text check
FUNCTION IsTextFile (sFileName AS STRING) As Integer
	DIM sBuffer AS STRING, sChar AS INTEGER
	DIM lLastPos AS LONG, i AS INTEGER
	DIM iTHandle AS INTEGER, iBytesRead AS INTEGER

	iTHandle = FreeFile()
	If (Open(sFileName FOR BINARY Access Read As iTHandle) <> 0) Then Return FALSE

	sBuffer = Space(16384)

	PRINT "Scanning "; sFileName; " ..."
	WHILE (NOT EOF(iTHandle))
		' Read from source, noting the number of bytes read
		lLastPos = LOC(iTHandle)
		GET #iTHandle, , sBuffer
		iBytesRead = LOC(iTHandle) - lLastPos

		LOCATE , 1
		PRINT USING "###% completed."; 100& * LOC(iTHandle) \ LOF(iTHandle);

		' Resize buffer to the number of bytes read from source
		sBuffer = Left(sBuffer, iBytesRead)

		' Test the content
		FOR i = 1 TO iBytesRead
			sChar = ASC(Mid(sBuffer, i, 1))
			IF (sChar < 32 AND sChar <> 9 AND sChar <> 10 AND sChar <> 13) THEN
				LOCATE , 1
				PRINT sFileName; " is not a text file!"
				IsTextFile = FALSE
				CLOSE iTHandle
				EXIT FUNCTION
			END IF
		NEXT
	WEND
	LOCATE , 1
	PRINT "Finished scanning."

	IsTextFile = TRUE
	CLOSE iTHandle
END FUNCTION

' Cleans text file
SUB TextClean (sFileName AS STRING)
	DIM sBuffer1 AS STRING, sBuffer2 AS STRING
	DIM lLastPos AS LONG, iHandleD AS INTEGER
	DIM sTempFile AS STRING, iHandleS AS INTEGER
	DIM iBytesRead AS INTEGER, i AS INTEGER
	DIM sChar AS INTEGER

	sTempFile = TempFile

	' Check if source file is present
	' If not then this causes a user trapable error
	iHandleS = FREEFILE
	OPEN sFileName FOR INPUT AS iHandleS
	CLOSE iHandleS

	' Reopen it for the real job
	OPEN sFileName FOR BINARY AS iHandleS

	' Overwrite distination file
	iHandleD = FREEFILE
	OPEN sTempFile FOR OUTPUT AS iHandleD
	CLOSE iHandleD

	' Reopen it for the real job
	OPEN sTempFile FOR BINARY AS iHandleD

	' Allocate buffer memory
	sBuffer1 = Space(16384)

	' Start copying the file
	PRINT "Removing invalid characters from "; sFileName; " ..."
	WHILE (NOT EOF(iHandleS))
		' Read from source, noting the number of bytes read
		lLastPos = LOC(iHandleS)
		GET #iHandleS, , sBuffer1
		iBytesRead = LOC(iHandleS) - lLastPos

		LOCATE , 1
		PRINT USING "###% completed."; 100& * LOC(iHandleS) \ LOF(iHandleS);

		' Resize buffer to the number of bytes read from source
		sBuffer1 = Left(sBuffer1, iBytesRead)

		' Remove crap
		sBuffer2 = ""
		FOR i = 1 TO iBytesRead
			sChar = ASC(Mid(sBuffer1, i, 1))
			SELECT CASE sChar
				CASE IS = 9, IS = 10, IS = 13
					sBuffer2 = sBuffer2 + Chr(sChar)
				CASE IS < 32
					' do nothing
				CASE ELSE
					sBuffer2 = sBuffer2 + Chr(sChar)
			END SELECT
		NEXT

		' Write the buffer content to the destination file
		PUT #iHandleD, , sBuffer2
	WEND
	LOCATE , 1
	PRINT "Finished removing invalid characters."

	CLOSE iHandleS, iHandleD
	FileCopy sTempFile, sFileName
	KILL sTempFile
END SUB

' Condenses the source code to use minimum disk space
SUB TextCondense (sFileName AS STRING)
	DIM sText AS STRING, iOHandle AS INTEGER
	DIM lTotalLines AS LONG, lActualLines AS LONG
	DIM sTempFile AS STRING, iIHandle AS INTEGER

	iIHandle = FREEFILE

	' Open file for counting the effective lines
	OPEN sFileName FOR INPUT AS iIHandle

	PRINT "Scanning "; sFileName; " ..."
	DO WHILE NOT EOF(iIHandle)
		LINE INPUT #iIHandle, sText
		lTotalLines = lTotalLines + 1
		IF Trim(sText) <> "" THEN lActualLines = lTotalLines
		LOCATE , 1
		PRINT USING "###% completed."; 128& * 100& * LOC(iIHandle) \ LOF(iIHandle);
	LOOP
	LOCATE , 1
	PRINT "Finished scanning."

	CLOSE iIHandle

	' Open file for the actual condensation
	OPEN sFileName FOR INPUT AS iIHandle

	iOHandle = FREEFILE
	sTempFile = TempFile
	OPEN sTempFile FOR OUTPUT AS iOHandle

	PRINT "Condensing "; sFileName; " ..."
	FOR lTotalLines = 1 TO lActualLines
		LINE INPUT #iIHandle, sText
		PRINT #iOHandle, RTrim(sText)
		LOCATE , 1
		PRINT USING "###% completed."; 100& * lTotalLines \ lActualLines;
	NEXT
	LOCATE , 1
	PRINT "Finished condensing."

	CLOSE iIHandle, iOHandle
	FileCopy sTempFile, sFileName
	KILL sTempFile
END SUB

' Tabifies a text file
SUB TextSpaceCompress (sFileName AS STRING, iLen AS INTEGER)
	DIM sIText AS STRING, iOHandle AS INTEGER
	DIM sTempFile AS STRING, iIHandle AS INTEGER
	DIM sOText AS STRING, i AS INTEGER, j AS INTEGER, sStr AS STRING

	' Open file for compressing spaces
	iIHandle = FREEFILE
	OPEN sFileName FOR INPUT AS iIHandle

	iOHandle = FREEFILE
	sTempFile = TempFile
	OPEN sTempFile FOR OUTPUT AS iOHandle

	PRINT "Compressing spaces to tabs ("; Trim(Str(iLen)); ":1) in "; sFileName; " ..."
	DO WHILE NOT EOF(iIHandle)
		LINE INPUT #iIHandle, sIText

		sOText = ""
		j = LEN(sIText)
		FOR i = 1 TO (j - iLen + 1) STEP iLen
			sStr = Mid(sIText, i, iLen)
			IF (sStr = Space(iLen)) THEN
				sOText = sOText + Chr(9)
			ELSE
				sOText = sOText + sStr
			END IF
		NEXT

		' Copy the remaining characters
		sOText = sOText + Right(sIText, j MOD iLen)

		PRINT #iOHandle, sOText

		LOCATE , 1
		PRINT USING "###% completed."; 128& * 100& * LOC(iIHandle) \ LOF(iIHandle);
	LOOP
	LOCATE , 1
	PRINT "Finished compressing spaces to tabs."

	CLOSE iIHandle, iOHandle
	FileCopy sTempFile, sFileName
	KILL sTempFile
END SUB

' Expands tabs to spaces
SUB TextTabExpand (sFileName AS STRING, iLen AS INTEGER)
	DIM sBuffer1 AS STRING, sBuffer2 AS STRING
	DIM lLastPos AS LONG, iHandleD AS INTEGER
	DIM sTempFile AS STRING, iHandleS AS INTEGER
	DIM iBytesRead AS INTEGER, i AS INTEGER
	DIM sChar AS STRING * 1

	sTempFile = TempFile

	' Check if source file is present
	' If not then this causes a user trapable error
	iHandleS = FREEFILE
	OPEN sFileName FOR INPUT AS iHandleS
	CLOSE iHandleS

	' Reopen it for the real job
	OPEN sFileName FOR BINARY AS iHandleS

	' Overwrite distination file
	iHandleD = FREEFILE
	OPEN sTempFile FOR OUTPUT AS iHandleD
	CLOSE iHandleD

	' Reopen it for the real job
	OPEN sTempFile FOR BINARY AS iHandleD

	' Allocate buffer memory
	sBuffer1 = Space(16384)

	' Start copying the file
	PRINT "Expanding tabs to spaces (1:"; Trim(Str(iLen)); ") in "; sFileName; " ..."
	WHILE (NOT EOF(iHandleS))
		' Read from source, noting the number of bytes read
		lLastPos = LOC(iHandleS)
		GET #iHandleS, , sBuffer1
		iBytesRead = LOC(iHandleS) - lLastPos

		LOCATE , 1
		PRINT USING "###% completed."; 100& * LOC(iHandleS) \ LOF(iHandleS);

		' Resize buffer to the number of bytes read from source
		sBuffer1 = Left(sBuffer1, iBytesRead)

		' Expand
		sBuffer2 = ""
		FOR i = 1 TO iBytesRead
			sChar = Mid(sBuffer1, i, 1)
			IF (sChar = Chr(9)) THEN
				sBuffer2 = sBuffer2 + SPACE(iLen)
			ELSE
				sBuffer2 = sBuffer2 + sChar
			END IF
		NEXT

		' Write the buffer content to the destination file
		PUT #iHandleD, , sBuffer2
	WEND
	LOCATE , 1
	PRINT "Finished expanding tabs to spaces."

	CLOSE iHandleS, iHandleD
	FileCopy sTempFile, sFileName
	KILL sTempFile
END SUB

' Check the command line and then collect relevant data
If (Command = "" OR ArgvPresent("?")) THEN
	PRINT
	PRINT "Text file condenser. Version 4.0"
	PRINT
	PRINT "Copyright (c) Samuel Gomes, 1998-2010."
	PRINT "All rights reserved."
	PRINT
	PRINT "mailto: v_2samg@hotmail.com"
	PRINT
	PRINT "Usage: CONDENSE [InFile] [/B] [/C] [/X] [/S[4 | 8] [/T[4 | 8] [/K] [/?]"
	PRINT "    InFile          is the input file"
	PRINT "    /B              backups input text file"
	PRINT "    /C              strips invalid characters"
	PRINT "    /X              avoids extra text file checks"
	PRINT "    /S              no tab to space expansion"
	PRINT "    /S4             expands 1 tab to 4 spaces (default)"
	PRINT "    /S8             expands 1 tab to 8 spaces"
	PRINT "    /T              no space to tab compression"
	PRINT "    /T4             compresses 4 spaces to 1 tab (default)"
	PRINT "    /T8             compresses 8 spaces to 1 tab"
	PRINT "    /K              performs a text file check only"
	PRINT "    /?              shows this help message"
	End
END IF

' Main
DIM sTextFile AS STRING
DIM lTextFileSizeOld AS LONG, lTextFileSizeNew AS Long

' Resolve the input file name
sTextFile = Command(1)

' Perform solitary text file check if specified
If (ArgvPresent("K")) THEN
	IF (IsTextFile(sTextFile)) THEN
		PRINT sTextFile; " is a text file."
	END IF

	End
End IF

' Note original file size
lTextFileSizeOld = CLng(FileLen(sTextFile))

' Backup input file if specified
IF (ArgvPresent("B")) THEN
	FileCopy sTextFile, sTextFile + ".bak"
END IF

' Check text file
IF (NOT ArgvPresent("C") AND NOT ArgvPresent("X")) THEN
	IF (NOT IsTextFile(sTextFile)) THEN
		End
	END IF
END IF

' Strip invalid characters
IF (ArgvPresent("C")) THEN
	TextClean sTextFile
END IF

' Expand tabs to spaces
IF (ArgvPresent("S8")) THEN
	TextTabExpand sTextFile, 8
ELSEIF (ArgvPresent("S")) THEN
	' no expansion
ELSE
	TextTabExpand sTextFile, 4
END IF

' Condense it
TextCondense sTextFile

' Compress spaces to tabs
IF (ArgvPresent("T8")) THEN
	TextSpaceCompress sTextFile, 8
ELSEIF (ArgvPresent("T")) THEN
	' no compression
ELSE
	TextSpaceCompress sTextFile, 4
END IF

' Get new file size
lTextFileSizeNew = CLng(FileLen(sTextFile))

' Print some statistics
PRINT
PRINT "Original size:"; lTextFileSizeOld; "bytes"
PRINT "Current size:"; lTextFileSizeNew; "bytes"
PRINT "Condensation: "; Trim(Str(100 - INT(100 * (lTextFileSizeNew / lTextFileSizeOld)))); "%"

End
