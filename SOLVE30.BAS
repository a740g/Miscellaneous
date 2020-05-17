' Finds a 3x3 matrix where adding 3 numbers horizontally, verticall or diagonally results in the number 30
DECLARE SUB printMatrix ()
DECLARE SUB clearMatrix ()
DECLARE FUNCTION checkMatrix () As Integer
DECLARE FUNCTION isValueInMatrix (i As Integer) As Integer

' 6 to 14 add 30
CONST TSUM = 30
CONST TSTART = 6

DIM SHARED matrix(1 TO 3, 1 TO 3) AS INTEGER
DIM values(1 TO 9) AS INTEGER
DIM i AS INTEGER, v AS INTEGER
DIM x AS INTEGER, y AS INTEGER

RANDOMIZE TIMER

CLS

FOR i = 0 TO 8
    values(i + 1) = TSTART + i
NEXT


DO
    clearMatrix

    FOR y = 1 TO 3
        FOR x = 1 TO 3
            DO
                v = values(INT(RND * 9) + 1)
            LOOP WHILE isValueInMatrix(v)
           
            matrix(x, y) = v
        NEXT
        ' Quit if the first step is wrong!
        IF (matrix(1, y) + matrix(2, y) + matrix(3, y) <> TSUM) THEN EXIT FOR
    NEXT

    printMatrix
LOOP WHILE checkMatrix = 0

PRINT : PRINT
PRINT "Solved!"

END

FUNCTION checkMatrix () As Integer
    DIM i AS INTEGER

    FOR i = 1 TO 3
        IF (matrix(1, i) + matrix(2, i) + matrix(3, i) <> TSUM) THEN
            checkMatrix = 0
            EXIT FUNCTION
        END IF
    NEXT
   
    FOR i = 1 TO 3
        IF (matrix(i, 1) + matrix(i, 2) + matrix(i, 3) <> TSUM) THEN
            checkMatrix = 0
            EXIT FUNCTION
        END IF
    NEXT

    IF (matrix(1, 1) + matrix(2, 2) + matrix(3, 3) <> TSUM) THEN
        checkMatrix = 0
        EXIT FUNCTION
    END IF

    IF (matrix(1, 3) + matrix(2, 2) + matrix(3, 1) <> TSUM) THEN
        checkMatrix = 0
        EXIT FUNCTION
    END IF

    checkMatrix = -1
END FUNCTION

SUB clearMatrix
    DIM x AS INTEGER, y AS INTEGER
   
    FOR y = 1 TO 3
        FOR x = 1 TO 3
            matrix(x, y) = 0
        NEXT
    NEXT
END SUB

FUNCTION isValueInMatrix (i As Integer) As Integer
    DIM x AS INTEGER, y AS INTEGER
  
    FOR y = 1 TO 3
        FOR x = 1 TO 3
            IF (matrix(x, y) = i) THEN
                isValueInMatrix = -1
                EXIT FUNCTION
            END IF
        NEXT
    NEXT

    isValueInMatrix = 0
END FUNCTION

SUB printMatrix
    DIM x AS INTEGER, y AS INTEGER

    LOCATE 1, 1

    FOR y = 1 TO 3
        FOR x = 1 TO 3
            PRINT matrix(x, y),
        NEXT
        PRINT : PRINT
    NEXT
END SUB

