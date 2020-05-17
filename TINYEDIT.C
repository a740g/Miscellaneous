/* Tiny text editor */
/* Created 25/03/2004 Blade */

/* Standard include files */
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>

/* Keyboard	key	codes */
#define	KB_NOKEY 0
#define	KB_ESC 27
#define	KB_BACKSPACE 8
#define	KB_ENTER 13
#define	KB_TAB 9
#define	KB_LEFT	331
#define	KB_RIGHT 333
#define	KB_INSERT 338
#define	KB_DELETE 339
#define	KB_HOME	327
#define	KB_END 335

/* Carriage return - line feed pair */
#define	CRLF "\r\n"

/* Something to make life easier */
typedef unsigned short word;
typedef int bool;
const true = 1;
const false = 0;

/* Global text buffer */
char **text = NULL;
char lines = 0;
const lineCharMax = 80;

/* Dynamically allocates memory for another line */
bool addLine(void) {
	/* resize the lines array */
	text = realloc(text, (lines + 1) * sizeof(char **));
	if (text == NULL) return false;

	/* allocate a new line */
	text[lines] = calloc(lineCharMax, sizeof(char));
	if (text == NULL) return false;

	lines++;

	/* First line default */
	if (lines == 1) {
		strcpy(text[0], "Start entering text here. Press 'ENTER' for next line and 'ESC' when done...");
	}

	return true;
}

/* Print the character to the screen with the correct color */
void printChar(char c) {
	/* choose the correct color for a type of character */
	if (isdigit(c)) {
		textcolor(BLUE);
	}
	else if (ispunct(c)) {
		textcolor(MAGENTA);
	}
	else if (isupper(c)) {
		textcolor(GREEN);
	}
	else if (islower(c)) {
		textcolor(CYAN);
	}
	else {
		textcolor(LIGHTGRAY);
	}

	putch(c);
}

/* Print the whole string to the dislay */
void printString(char *s) {
	int i;

	for (i = 0; s[i] != 0; i++) {
		printChar(s[i]);
	}
}

/* Gets input from the keyboard; also return extended input */
word getInput(void) {
	word c = KB_NOKEY;

	if (kbhit()) {
		c = getch();
		if (!c) {
			c = 256 + getch();
		}
	}

	return c;
}

/* Edit and display a string on the screen */
bool editText(char *txt, int len) {
	int y, x;
	word inp;

	/* get the current line on the display */
	y = wherey();
	x = 1 + strlen(txt);

	/* display the initial string */
	gotoxy(1, y);
	printString(txt);
	_setcursortype(_SOLIDCURSOR);

	/* edit loop */
	do {
		inp = getInput();

		if (inp == KB_ENTER || inp == KB_ESC) {
			/* no op */
		}
		else if (inp == KB_BACKSPACE) {
			if (x > 1) {
				txt[strlen(txt) - 1] = 0;
				x--;
				gotoxy(x, y);
				putch(' ');
				gotoxy(x, y);
			}
		}
		else if (inp < 256 && inp > 0) {
			if (x < len) {
				txt[strlen(txt)] = inp;
				txt[strlen(txt) + 1] = 0;
				printChar(inp);
				x++;
			}
		}
	} while (inp != KB_ENTER && inp != KB_ESC);

	_setcursortype(_NOCURSOR);

	return (inp == KB_ENTER ? true : false);
}

/* Gets a bunch of text into the global text buffer */
void getText(void) {
	int y = 1;
	bool done;

	/* clear the screen */
	textmode(C80);
	clrscr();

	do {
		/* add a line of text to the global text buffer */
		if (!addLine()) {
			fputs("Out of memory!", stderr);
			exit(EXIT_FAILURE);
		}

		/* edit the line */
		done = !editText(text[lines - 1], lineCharMax);

		if (!done) {
			y++;
			if (y > 25) {
				movetext(1, 2, 80, 25, 1, 1);
				gotoxy(1, 25);
				clreol();
			}
			else {
				gotoxy(1, y);
			}
		}
	} while (!done);
}

/* Show some info on the text typed in */
void showTextStats(void) {
	int realLines = 0;
	long chars = 0;
	long realChars = 0;
	long digits = 0;
	long letters = 0;
	long uppr = 0;
	long lwer = 0;
	long puncts = 0;
	long spaces = 0;
	int i, j;

	/* Calculate the actual lines & characters typed */
	for (i = 0; i < lines; i++) {
		if (text[i][0] != 0) realLines++;

		for (j = 0; text[i][j] != 0; j++) {
			chars++;
			if (isdigit(text[i][j])) {
				digits++;
			}
			else if (isupper(text[i][j])) {
				uppr++;
			}
			else if (islower(text[i][j])) {
				lwer++;
			}
			else if (ispunct(text[i][j])) {
				puncts++;
			}
			else if (isspace(text[i][j])) {
				spaces++;
			}
		}
	}
	realChars = chars + lines;
	letters = uppr + lwer;

	cprintf(CRLF"Total lines = %d"CRLF
			"Actual lines = %d"CRLF
			"Total characters = %ld"CRLF
			"Actual characters = %ld"CRLF
			"Total digits = %ld"CRLF
			"Total alphabets = %ld"CRLF
			"Total lower case alphabets = %ld"CRLF
			"Total upper case alphabets = %ld"CRLF
			"Total punctuations = %ld"CRLF
			"Total spaces = %ld"CRLF,
			lines, realLines, chars, realChars, digits, letters, lwer, uppr, puncts, spaces);

	cputs("\n\rPress any key to continue...");
	while (getInput() == KB_NOKEY);
}

/* Program entry point */
void main() {
	/* get some text from the user */
	getText();

	/* show text statistics */
	showTextStats();
}
