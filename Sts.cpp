/****************************************************************************\
*                                                                            *
*	Share Transfer System. Version 1.5.2                                     *
*	Copyright (c) Samuel Gomes (Blade), 2001-2003.							 *
*   All rights reserved.                                                     *
*	mailto: blade_go@hotmail.com											 *
*                                                                            *
*	Compiles only with Borland/Turbo C++									 *
*                                                                            *
\****************************************************************************/

/****************************** Include files *******************************/
#include <conio.h>
#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <io.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/************************** Definitions and macros **************************/
#define LESSER (-1)
#define EQUAL 0
#define GREATER 1
#define EOS 0
#define NULL_STRING "\0"
#define FALSE 0
#define TRUE 1

/* Small utilities */
#define ISEVEN(_x_) (((_x_) >> 1) + ((_x_) >> 1) == (_x_))
#define SGN(_x_) (((_x_) == 0) ? EQUAL : (((_x_) > 0) ? GREATER : LESSER))
#define DIM(_x_) (sizeof(_x_) / sizeof(_x_[0]))
#define STS_ID(_a_, _b_, _c_, _d_) ((unsigned long)(_a_) | ((unsigned long)(_b_) << 8) | ((unsigned long)(_c_) << 16) | ((unsigned long)(_d_) << 24))
#define EMPTY_STRING(_s_) ((_s_) == NULL || (_s_)[0] == EOS)
#define CLEAR_VAR(__v) memset(&(__v), EOS, sizeof(__v))

/* Keyboard key codes */
#define kbNoKey 0
#define kbEsc 27
#define kbBackSpace 8
#define kbEnter 13
#define kbTab 9
#define kbF1 315
#define kbF2 316
#define kbF3 317
#define kbF4 318
#define kbF5 319
#define kbF6 320
#define kbF7 321
#define kbF8 322
#define kbF9 323
#define kbF10 324
#define kbF11 389
#define kbF12 390
#define kbUp 328
#define kbDown 336
#define kbLeft 331
#define kbRight 333
#define kbInsert 338
#define kbDelete 339
#define kbHome 327
#define kbEnd 335
#define kbPageUp 329
#define kbPageDown 337
#define kbShiftTab 271
#define kbCtrlA 1
#define kbCtrlB 2
#define kbCtrlC 3
#define kbCtrlD 4
#define kbCtrlE 5
#define kbCtrlF 6
#define kbCtrlG 7
#define kbCtrlH 8
#define kbCtrlI 9
#define kbCtrlJ 10
#define kbCtrlK 11
#define kbCtrlL 12
#define kbCtrlM 13
#define kbCtrlN 14
#define kbCtrlO 15
#define kbCtrlP 16
#define kbCtrlQ 17
#define kbCtrlR 18
#define kbCtrlS 19
#define kbCtrlT 20
#define kbCtrlU 21
#define kbCtrlV 22
#define kbCtrlW 23
#define kbCtrlX 24
#define kbCtrlY 25
#define kbCtrlZ 26
#define kbCtrlEnter 10
#define kbSpaceBar 32
#define kbCtrlBackSpace 127
#define kbCtrlF1 350
#define kbCtrlF2 351
#define kbCtrlF3 352
#define kbCtrlF4 353
#define kbCtrlF5 354
#define kbCtrlF6 355
#define kbCtrlF7 356
#define kbCtrlF8 357
#define kbCtrlF9 358
#define kbCtrlF10 359
#define kbCtrlF11 393
#define kbCtrlF12 394
#define kbShiftF1 340
#define kbShiftF2 341
#define kbShiftF3 342
#define kbShiftF4 343
#define kbShiftF5 344
#define kbShiftF6 345
#define kbShiftF7 346
#define kbShiftF8 347
#define kbShiftF9 348
#define kbShiftF10 349
#define kbShiftF11 391
#define kbShiftF12 392
#define kbAltF1 360
#define kbAltF2 361
#define kbAltF3 362
#define kbAltF4 363
#define kbAltF5 364
#define kbAltF6 365
#define kbAltF7 366
#define kbAltF8 367
#define kbAltF9 368
#define kbAltF10 369
#define kbAltF11 395
#define kbAltF12 396
#define kbAltMinus 386
#define kbAltPlus 387
#define kbAlt1 376
#define kbAlt2 377
#define kbAlt3 378
#define kbAlt4 379
#define kbAlt5 380
#define kbAlt6 381
#define kbAlt7 382
#define kbAlt8 383
#define kbAlt9 384
#define kbAlt0 385
#define kbAltA 286
#define kbAltB 304
#define kbAltC 302
#define kbAltD 288
#define kbAltE 274
#define kbAltF 289
#define kbAltG 290
#define kbAltH 291
#define kbAltI 279
#define kbAltJ 292
#define kbAltK 293
#define kbAltL 294
#define kbAltM 306
#define kbAltN 305
#define kbAltO 280
#define kbAltP 281
#define kbAltQ 272
#define kbAltR 275
#define kbAltS 287
#define kbAltT 276
#define kbAltU 278
#define kbAltV 303
#define kbAltW 273
#define kbAltX 301
#define kbAltY 277
#define kbAltZ 300

// Screen attributes
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Don't change this or you'll break the db core code
#define MY_STRING_MAX 81

// Shareholder categories
#define CATEGORY_RI 0
#define CATEGORY_NRI 1
#define CATEGORY_DC 2
#define CATEGORY_NRC 3

// Files used by our application
#define SHARES_FILE "shares.sts"
#define TRANSFER_FILE "transfer.sts"
#define TEMP_FILE "sts.tmp"

/***************************** Type definitions *****************************/
// The following two structs MUST be packed since they are used to save
// data to disk.

// Byte packing
#pragma option -a-

// Transfer register data format
struct TransferData_t {
	long Serial;
	date Date;
	long SellerFolio;
	long BuyerFolio;
	long Certificate;
	long Shares;
};

struct ShareData_t {
	long Folio;
	long Certificate;
	char Name1[MY_STRING_MAX];
	char Name2[MY_STRING_MAX];
	char Name3[MY_STRING_MAX];
	char Address[MY_STRING_MAX];
	char City[MY_STRING_MAX];
	long PinCode;
	char State[MY_STRING_MAX];
	short Category;
	long Shares;
};

/********************** Function prototypes and classes *********************/
/* Friendly debugging aids */
#define Assert(_e_) ((_e_) ? (void)0 : (void)_AssertFail(#_e_, __FILE__, __LINE__))
#define DebugPrint(_s_) fputs(_s_, stderr)

// Keyboard class; pretty simple hu?
class Keyboard_t {
public:
	Keyboard_t();
	int Waiting();
	int Input();
	void WaitPress();
	void Clear();
};

// Window class; simply sucks!
class Window_t {
	int Left;
	int Top;
	int Right;
	int Bottom;
	char Caption[MY_STRING_MAX];
	void MenuDraw(char *options[], int count, int x, int y, int foreCol, int backCol, int spacing, int selected);
public:
	// Constructor
	Window_t();
	void SetBounds(int left, int top, int right, int bottom);
	void SetCaption(char *caption);
	void SetColors(int foreground, int background);
	void SetCursorType(int curType);
	void Draw();
	void Clear();
	void PutS(char *s);
	int PrintF(char *format, ...);
	void GoToXY(int x, int y);
	int WhereX();
	int WhereY();
	void SetFocus();
	int MaxX();
	int MaxY();
	void InputDrawField(char *s, int maxLen, int x, int y, int foreCol, int backCol);
	void Input(char *s, int maxLen, int x, int y, int foreCol, int backCol);
	void Input(long *i, int x, int y, int foreCol, int backCol);
	int Menu(char *options[], int count, int x, int y, int foreCol, int backCol, int spacing);
	void Reset();
	void Scroll(int flag);
	char Choice(char *prompt, char *choice, int x, int y, int foreCol, int backCol);
};

// Share register db class
class ShareRegister_t {
	FILE *fs;
	long CurrentRecord;
public:
	ShareRegister_t();
	~ShareRegister_t();
	void Add(ShareData_t *share);
	int Get(long record, ShareData_t *share);
	void Erase(long record);
	long Search(long certificate);
};

// Transfer register db class
class TransferRegister_t {
	FILE *fs;
	long CurrentRecord;
public:
	TransferRegister_t();
	~TransferRegister_t();
	void Add(TransferData_t *data);
	int Get(long record, TransferData_t *data);
	void Erase(long record);
};

// STS application class; makes our program more C++ ;)
class Application_t {
	int TranslateCategoryString(char *c);
	char *TranslateCategoryInt(int c);
	int MainMenu();
	int FileMaintainanceMenu();
	int ReportMenu();
	int ShareRegisterMenu();
	int TransferRegisterMenu();
	void ExitCredits();
	void DrawShareRegisterFields(Window_t &win, ShareData_t &data);
	void ShareRegisterAdd();
	void ShareRegisterErase();
	void ShareRegisterView();
	void DrawTransferSharesFields(Window_t &win, TransferData_t &data);
	void TransferShares();
	void GenerateReport();
public:
	Application_t();
	~Application_t();
	void Run();
};

/**************************** Globals variables *****************************/
int ErrorHandlerRegistered = FALSE;
/* Holds the error message */
char ErrorString[UCHAR_MAX + 1] = NULL_STRING"$Id: sts.cpp, v1.5 2003/01/28 20:00:04, blade exp $";
// Global share register
ShareRegister_t ShareRegister;
// Global transfer register
TransferRegister_t TransferRegister;

/*************************** Error handling code ****************************/
/* Will be called by atexit registered by Error_init on exit */
void Error_Done(void) {
	if (ErrorString[0] == EOS) return;

	/* Reset to default text mode */
	textmode(C80);
	_setcursortype(_NORMALCURSOR);
	textcolor(LIGHTGRAY);
	textbackground(BLACK);
	clrscr();

	/* Write the text */
	fputs("Abnormal program termination: ", stderr);
	fputs(ErrorString, stderr);
	fputs("!\n", stderr);
}

/* Initializes the error reporting system */
int Error_Init(void) {
	if (ErrorHandlerRegistered) return FALSE;
	ErrorHandlerRegistered = (!atexit(Error_Done));

	return (ErrorHandlerRegistered ? TRUE : FALSE);
}

/* User level function to give up! */
void Error_Abort(char *Cause, ...) {
	va_list argPtr;

	/* Render the error string */
	va_start(argPtr, Cause);
	vsprintf(ErrorString, Cause, argPtr);
	va_end(argPtr);

	/* If we are not registered lets call the routine ourself */
	if (!ErrorHandlerRegistered) Error_Done();
	exit(EXIT_FAILURE);
}

// Not for you!
void _AssertFail(char *__cond, char *__file, long __line) {
	Error_Abort("Assertion failed (%s) in file \"%s\" at line %lu", __cond, __file, __line);
}

/**************************** Miscellaneous code ****************************/
/* Standard min template */
template <class T> T min(T x, T y) {
	return ((x < y) ? x : y);
}

/* Standard max template */
template <class T> T max(T x, T y) {
	return ((x > y) ? x : y);
}

// Beeeep! ;)
void Beep() {
	sound(500);
	delay(50);
	nosound();
}

// Puts two numbers together in a single number
// Couldn't think of some other better way :(
long JoinLong(long a, long b) {
	char buffer[CHAR_MAX];

	sprintf(buffer, "%ld%ld", a, b);
	return atol(buffer);
}

/************************** Keyboard handling code **************************/
// Constructor
Keyboard_t::Keyboard_t() {
	Clear();
}

/* Returns true if a key is waiting in the keyboard buffer */
int Keyboard_t::Waiting() {
	return kbhit();
}

/* Retuns the code of a normal ascii or extended key press else 0 */
int Keyboard_t::Input() {
	int c = kbNoKey;

	if (kbhit()) {
		c = getch();
		if (!c) c = 256 + getch();
	}

	return c;
}

/* Waits for a keypress to occur */
void Keyboard_t::WaitPress() {
	while (!kbhit());
}

/* Empties the keyboard buffer */
void Keyboard_t::Clear() {
	while (Input() != kbNoKey);
}

/*************************** User interface code ****************************/
Window_t::Window_t() {
	SetCaption(NULL_STRING);			// nothing
	SetBounds(1, 1, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetColors(WHITE, BLACK);
}

void Window_t::SetBounds(int left, int top, int right, int bottom) {
	Left = left;
	Top = top;
	Right = right;
	Bottom = bottom;
}

void Window_t::SetCaption(char *caption) {
	strncpy(Caption, caption, sizeof(Caption) - 1);
}

void Window_t::SetColors(int foreground, int background) {
	textcolor(foreground);
	textbackground(background);
}

void Window_t::SetCursorType(int curType) {
	_setcursortype(curType);
}

void Window_t::Draw() {
	// Reset Window
	window(1, 1, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Turn scrolling off; fixes one nasty 'age-old' 'mis-feature'!
	Scroll(FALSE);

	// Draw the edges
	gotoxy(Right, Bottom);
	putch('Ù');
	gotoxy(Left, Bottom);
	putch('À');
	gotoxy(Left, Top);
	putch('Ú');
	gotoxy(Right, Top);
	putch('¿');

	// Draw the sides
	for (int x = Left + 1; x < Right; x++) {
		gotoxy(x, Top);
		putch('Ä');
		gotoxy(x, Bottom);
		putch('Ä');
	}
	for (int y = Top + 1; y < Bottom; y++) {
		gotoxy(Left, y);
		putch('³');
		gotoxy(Right, y);
		putch('³');
	}

	// Print the caption if it is non-empty
	if (strlen(Caption) > 0) {
		gotoxy((Right - Left - strlen(Caption) - 1) / 2 + Left, Top);
		putch(' ');
		cputs(Caption);
		putch(' ');
	}

	SetFocus();

	Scroll(TRUE);
}

char Window_t::Choice(char *prompt, char *choice, int x, int y, int foreCol, int backCol) {
	Keyboard_t kb;
	int inp = kbNoKey;
	int myX, myY;

	SetColors(foreCol, backCol);
	GoToXY(x, y);
	delline();		// required!
	PrintF("%s (%c/%c)? ", prompt, choice[0], choice[1]);
	myX = WhereX();
	myY = WhereY();

	SetCursorType(_NORMALCURSOR);

	while (inp != tolower(choice[0]) && inp != tolower(choice[1])) {
		if (inp >= 0 && inp <= 255) {
			GoToXY(myX, myY);
			PrintF("%c", inp);
		}

		kb.WaitPress();
		inp = kb.Input();
	}

	SetCursorType(_NOCURSOR);

	return tolower(inp);
}

void Window_t::Clear() {
	clrscr();
}

void Window_t::PutS(char *s) {
	cputs(s);
}

int Window_t::PrintF(char *format, ...) {
   va_list argptr;
   int cnt;
   char buffer[SCREEN_WIDTH * SCREEN_HEIGHT + 1];

   va_start(argptr, format);
   cnt = vsprintf(buffer, format, argptr);
   va_end(argptr);

   cputs(buffer);
   return cnt;
}

void Window_t::GoToXY(int x, int y) {
	gotoxy(x, y);
}

int Window_t::WhereX() {
	return wherex();
}

int Window_t::WhereY() {
	return wherey();
}

void Window_t::SetFocus() {
	window(Left + 1, Top + 1, Right - 1, Bottom - 1);
}

int Window_t::MaxX() {
	return Right - Left - 1;
}

int Window_t::MaxY() {
	return Bottom - Top - 1;
}

void Window_t::InputDrawField(char *s, int maxLen, int x, int y, int foreCol, int backCol) {
	GoToXY(x, y);
	SetColors(foreCol, backCol);
	for (int i = 0; i < maxLen + 1; i++) PutS(" ");
	GoToXY(x, y);
	PutS(s);
}

void Window_t::Input(char *s, int maxLen, int x, int y, int foreCol, int backCol) {
	char temp[UCHAR_MAX + 1];

	maxLen = min(255, maxLen + 1);
	temp[0] = maxLen;

	InputDrawField(s, maxLen - 1, x, y, foreCol, backCol);
	GoToXY(x, y);
	SetCursorType(_NORMALCURSOR);

	strcpy(s, cgets(temp));

	SetCursorType(_NOCURSOR);
}

void Window_t::Input(long *i, int x, int y, int foreCol, int backCol) {
	char temp[CHAR_MAX];

	ltoa(*i, temp, 10);
	Input(temp, 10, x, y, foreCol, backCol);
	*i = atol(temp);
}

// Private method
void Window_t::MenuDraw(char *options[], int count, int x, int y, int foreCol, int backCol, int spacing, int selected) {
	int i, j = 0;

	Scroll(FALSE);	// bugfix

	for (i = 0; i < count; i++) {
		if (i == selected)
			SetColors(backCol, foreCol);
		else
			SetColors(foreCol, backCol);
		GoToXY(x, y + j);
		PutS(options[i]);
		j += spacing;
	}

	Scroll(TRUE);
}

int Window_t::Menu(char *options[], int count, int x, int y, int foreCol, int backCol, int spacing) {
	Keyboard_t kb;
	int inp = kbNoKey;
	int m = 0;

	while (inp != kbEnter && inp != kbEsc) {
		MenuDraw(options, count, x, y, foreCol, backCol, spacing, m);

		kb.WaitPress();
		inp = kb.Input();

		switch (inp) {
			case kbUp:
				m--;
				if (m < 0) m = count - 1;
				break;
			case kbDown:
				m++;
				if (m >= count) m = 0;
				break;
			case kbEnter:
				// nop
				break;
			case kbEsc:
				m = -kbEsc;
				break;
			default:
				Beep();
		}
	}

	return m;
}

void Window_t::Reset() {
	SetCaption(NULL_STRING);			// nothing
	SetBounds(1, 1, SCREEN_WIDTH, SCREEN_HEIGHT);
	window(1, 1, SCREEN_WIDTH, SCREEN_HEIGHT);		// must do this explicitly!
	SetColors(WHITE, BLACK);
}

void Window_t::Scroll(int flag) {
	_wscroll = flag;
}

/**************************** Core database code ****************************/
// Constructor
ShareRegister_t::ShareRegister_t() {
	// Create file if it does not exist
	if (access(SHARES_FILE, 0) != 0) {
		Assert((fs = fopen(SHARES_FILE, "wb")) != NULL);
		Assert(fclose(fs) == 0);
	}

	Assert((fs = fopen(SHARES_FILE, "r+b")) != NULL);
	CurrentRecord = 0;
}

// Destructor
ShareRegister_t::~ShareRegister_t() {
	Assert(fclose(fs) == 0);
};

// Reads the specified record from a database
int ShareRegister_t::Get(long record, ShareData_t *share) {
	long dboffset = sizeof(ShareData_t) * record;

	fflush(fs);
	if (fseek(fs, dboffset, SEEK_SET) != 0) return FALSE;

	if (fread((char *)share, 1, sizeof(ShareData_t), fs) == sizeof(ShareData_t)) {
		CurrentRecord = record;
		return TRUE;
	}

	return FALSE;
}

// Searches specified record by certificate no.
// If the record is found all relevant data is placed in the Share member
long ShareRegister_t::Search(long certificate) {
	ShareData_t share;
	long rec = 0;

	while (Get(rec, &share)) {
		if (certificate == share.Certificate) {
			return rec;
		}
		rec++;
	}

	return -1;
}

void ShareRegister_t::Erase(long record) {
	FILE *temp;
	long rec = 0;
	ShareData_t share;

	Assert((temp = fopen(TEMP_FILE, "wb")) != NULL);

	while (Get(rec, &share)) {
		if (rec != record) {
			Assert(fwrite((char *)&share, 1, sizeof(ShareData_t), temp) == sizeof(ShareData_t));
		}
		rec++;
	}

	Assert(fclose(temp) == 0);
	Assert(fclose(fs) == 0);
	Assert(unlink(SHARES_FILE) == 0);
	Assert(rename(TEMP_FILE, SHARES_FILE) == 0);

	Assert((fs = fopen(SHARES_FILE, "r+b")) != NULL);
	CurrentRecord = 0;
}

// Adds data in Share member to database
void ShareRegister_t::Add(ShareData_t *share) {
	ShareData_t temp;
	long record = 0;

	fflush(fs);
	while (Get(record, &temp)) record++;
	Assert(fseek(fs, record * sizeof(ShareData_t), SEEK_SET) == 0);	// seek to the correct offset

	CurrentRecord = record;
	Assert(fwrite((char *)share, 1, sizeof(ShareData_t), fs) == sizeof(ShareData_t));
}

// Constructor
TransferRegister_t::TransferRegister_t() {
	// Create file if it does not exist
	if (access(TRANSFER_FILE, 0) != 0) {
		Assert((fs = fopen(TRANSFER_FILE, "wb")) != NULL);
		Assert(fclose(fs) == 0);
	}

	Assert((fs = fopen(TRANSFER_FILE, "r+b")) != NULL);
	CurrentRecord = 0;
}

// Destructor
TransferRegister_t::~TransferRegister_t() {
	Assert(fclose(fs) == 0);
};

// Reads the specified record from a database
int TransferRegister_t::Get(long record, TransferData_t *data) {
	long dboffset = sizeof(TransferData_t) * record;

	fflush(fs);
	if (fseek(fs, dboffset, SEEK_SET) != 0) return FALSE;

	if (fread((char *)data, 1, sizeof(TransferData_t), fs) == sizeof(TransferData_t)) {
		CurrentRecord = record;
		return TRUE;
	}

	return FALSE;
}

void TransferRegister_t::Erase(long record) {
	FILE *temp;
	long rec = 0;
	TransferData_t trans;

	Assert((temp = fopen(TEMP_FILE, "wb")) != NULL);

	while (Get(rec, &trans)) {
		if (rec != record) {
			Assert(fwrite((char *)&trans, 1, sizeof(TransferData_t), temp) == sizeof(TransferData_t));
		}
		rec++;
	}

	Assert(fclose(temp) == 0);
	Assert(fclose(fs) == 0);
	Assert(unlink(TRANSFER_FILE) == 0);
	Assert(rename(TEMP_FILE, TRANSFER_FILE) == 0);

	Assert((fs = fopen(TRANSFER_FILE, "r+b")) != NULL);
	CurrentRecord = 0;
}

// Adds data from the database
void TransferRegister_t::Add(TransferData_t *data) {
	TransferData_t temp;
	long record = 0;

	fflush(fs);
	while (Get(record, &temp)) record++;
	Assert(fseek(fs, record * sizeof(TransferData_t), SEEK_SET) == 0);	// seek to the correct offset

	CurrentRecord = record;
	Assert(fwrite((char *)data, 1, sizeof(TransferData_t), fs) == sizeof(TransferData_t));
}

/******************************** Main code *********************************/

Application_t::Application_t() {
	// Initialize the error reporting system
	Assert(Error_Init());

	textmode(C80);
}

Application_t::~Application_t() {
	ExitCredits();
}

int Application_t::TranslateCategoryString(char *c) {
	if (stricmp(c, "RI") == EQUAL)
		return CATEGORY_RI;
	else if (stricmp(c, "NRI") == EQUAL)
		return CATEGORY_NRI;
	else if (stricmp(c, "DC") == EQUAL)
		return CATEGORY_DC;
	else if (stricmp(c, "NRC") == EQUAL)
		return CATEGORY_NRC;
	else
		return -1;
}

char *Application_t::TranslateCategoryInt(int c) {
	if (c == CATEGORY_RI)
		return "RI";
	else if (c == CATEGORY_NRI)
		return "NRI";
	else if (c == CATEGORY_DC)
		return "DC";
	else if (c == CATEGORY_NRC)
		return "NRC";
	else
		return "";
}

void Application_t::Run() {
	int m = -1;	// menu return code

	// Note: we set m to -1 so that later whiles and switches don't get
	// default values 'just like that'! ;)
	while (m != 2 && m != -kbEsc) {
		m = MainMenu();

		switch (m) {
			case 0:
				while (m != 2 && m != -kbEsc) {
					m = FileMaintainanceMenu();

					switch (m) {
						case 0:
							while (m != 3 && m != -kbEsc) {
								m = ShareRegisterMenu();

								switch (m) {
									case 0:		// add
										ShareRegisterAdd();
										break;
									case 1:		// delete
										ShareRegisterErase();
										break;
									case 2:
										ShareRegisterView();
										break;
									case 3:		// exit menu
										break;
									default:
										Beep();
								}
							}
							m = -1;
							break;
						case 1:
							m = -1;		// bugfix
							while (m != 1 && m != -kbEsc) {
								m = TransferRegisterMenu();

								switch (m) {
									case 0:		// transfer
										TransferShares();
										break;
									case 1:		// exit
										break;
									default:
										Beep();
								}
							}
							m = -1;
							break;
						case 2:
							break;
						default:
							Beep();
					}
				}
				m = -1;
				break;
			case 1:
				m = -1;	// bugfix
				while (m != 1 && m != -kbEsc) {
					m = ReportMenu();

					switch (m) {
						case 0:		// report
							GenerateReport();
							break;
						case 1:
							break;
						default:
							Beep();
					}
				}
				m = -1;
				break;
			case 2:
				break;
			default:
				Beep();
		}
	}
}

// Main menu
int Application_t::MainMenu() {
	Window_t win;
	char *menu[] = {
		" 1> File Maintainance ",
		" 2> Report            ",
		" 3> Exit System       ",
	};

	win.SetCursorType(_NOCURSOR);
	win.SetColors(LIGHTGREEN, BLUE);
	win.SetCaption("Share Transfer System");
	win.Draw();
	win.Clear();
	win.SetCaption("Main Menu");
	win.SetBounds(28, 7, 53, 19);
	win.SetColors(YELLOW, BLACK);
	win.Draw();
	win.Clear();
	int m = win.Menu(menu, 3, 2, 2, WHITE, BLACK, 4);
	win.Reset();

	return m;
}

int Application_t::FileMaintainanceMenu() {
	Window_t win;
	char *menu[] = {
		" 1> Share Register      ",
		" 2> Transfer Register   ",
		" 3> Return To Main Menu ",
	};

	win.SetCursorType(_NOCURSOR);
	win.SetColors(LIGHTGREEN, BLUE);
	win.SetCaption("Share Transfer System");
	win.Draw();
	win.Clear();
	win.SetCaption("File Maintainance Menu");
	win.SetBounds(27, 7, 54, 19);
	win.SetColors(YELLOW, BLACK);
	win.Draw();
	win.Clear();
	int m = win.Menu(menu, 3, 2, 2, WHITE, BLACK, 4);
	win.Reset();

	return m;
}

int Application_t::ReportMenu() {
	Window_t win;
	char *menu[] = {
		" 1> Generate Report     ",
		" 2> Return To Main Menu ",
	};

	win.SetCursorType(_NOCURSOR);
	win.SetColors(LIGHTGREEN, BLUE);
	win.SetCaption("Share Transfer System");
	win.Draw();
	win.Clear();
	win.SetCaption("Report Menu");
	win.SetBounds(27, 9, 54, 17);
	win.SetColors(YELLOW, BLACK);
	win.Draw();
	win.Clear();
	int m = win.Menu(menu, 2, 2, 2, WHITE, BLACK, 4);
	win.Reset();

	return m;
}

int Application_t::ShareRegisterMenu() {
	Window_t win;
	char *menu[] = {
		" 1> Add Records                      ",
		" 2> Delete Records                   ",
		" 3> View Records                     ",
		" 4> Return To File Maintainance Menu ",
	};

	win.SetCursorType(_NOCURSOR);
	win.SetColors(LIGHTGREEN, BLUE);
	win.SetCaption("Share Transfer System");
	win.Draw();
	win.Clear();
	win.SetCaption("Share Register Menu");
	win.SetBounds(20, 5, 60, 21);
	win.SetColors(YELLOW, BLACK);
	win.Draw();
	win.Clear();
	int m = win.Menu(menu, 4, 2, 2, WHITE, BLACK, 4);
	win.Reset();

	return m;
}

int Application_t::TransferRegisterMenu() {
	Window_t win;
	char *menu[] = {
		" 1> Transfer Shares                  ",
		" 2> Return To File Maintainance Menu ",
	};

	win.SetCursorType(_NOCURSOR);
	win.SetColors(LIGHTGREEN, BLUE);
	win.SetCaption("Share Transfer System");
	win.Draw();
	win.Clear();
	win.SetCaption("Transfer Register Menu");
	win.SetBounds(20, 9, 60, 17);
	win.SetColors(YELLOW, BLACK);
	win.Draw();
	win.Clear();
	int m = win.Menu(menu, 2, 2, 2, WHITE, BLACK, 4);
	win.Reset();

	return m;
}

void Application_t::ExitCredits() {
	Window_t win;

	win.Clear();
	win.SetBounds(1, 1, SCREEN_WIDTH, 24);
	win.SetColors(LIGHTCYAN, BLACK);
	win.SetCaption("Credits");
	win.Draw();
	win.Clear();
	win.SetColors(LIGHTGREEN, BLACK);
	win.PutS("\n\n\n\nProgrammer:\n");
	win.PutS("Samuel Gomes (a.k.a. Blade)\n");
	win.PutS("\n\n\n\n\rCopyright (c) Samuel Gomes, 2001-2003.\n");
	win.PutS("All rights reserved.\n");
	win.PutS("\n\n\n\n\rmailto: blade_go@hotmail.com\n");
	win.PutS("mailto: blade_g@rediffmail.com");
	win.Reset();
	win.GoToXY(1, 24);
	win.SetCursorType(_NORMALCURSOR);
}

void Application_t::DrawShareRegisterFields(Window_t &win, ShareData_t &data) {
	date today;
	char temp[CHAR_MAX];

	getdate(&today);

	win.SetColors(LIGHTMAGENTA, BLUE);
	win.GoToXY(61, 1);
	win.PrintF("Date: %i/%i/%i", (int)today.da_day, (int)today.da_mon, today.da_year);
	win.SetColors(YELLOW, BLUE);
	win.GoToXY(29, 2);
	win.PrintF("           ");
	win.GoToXY(3, 2);
	win.PrintF("            Folio number: S%ld", data.Folio);
	win.GoToXY(29, 4);
	win.PrintF("          ");
	win.GoToXY(3, 4);
	win.PrintF("      Certificate number: %ld", data.Certificate);
	win.GoToXY(3, 6);
	win.PutS("      Shareholder name 1:");
	win.GoToXY(3, 8);
	win.PutS("      Shareholder name 2:");
	win.GoToXY(3, 10);
	win.PutS("      Shareholder name 3:");
	win.GoToXY(3, 12);
	win.PutS("                 Address:");
	win.GoToXY(3, 14);
	win.PutS("                    City:");
	win.GoToXY(3, 16);
	win.PutS("                 Pincode:");
	win.GoToXY(3, 18);
	win.PutS("                   State:");
	win.GoToXY(3, 20);
	win.PutS("Category (RI/NRI/NRC/DC):");
	win.GoToXY(3, 22);
	win.PutS("        Number of shares:");

	win.InputDrawField(data.Name1, 40, 29, 6, WHITE, GREEN);
	win.InputDrawField(data.Name2, 40, 29, 8, WHITE, GREEN);
	win.InputDrawField(data.Name3, 40, 29, 10, WHITE, GREEN);
	win.InputDrawField(data.Address, 40, 29, 12, WHITE, GREEN);
	win.InputDrawField(data.City, 20, 29, 14, WHITE, GREEN);
	win.InputDrawField(ltoa(data.PinCode, temp, 10), 10, 29, 16, WHITE, GREEN);
	win.InputDrawField(data.State, 20, 29, 18, WHITE, GREEN);
	win.InputDrawField(TranslateCategoryInt(data.Category), 3, 29, 20, WHITE, GREEN);
	win.InputDrawField(ltoa(data.Shares, temp, 10), 10, 29, 22, WHITE, GREEN);
}

void Application_t::ShareRegisterAdd() {
	char category[4];
	Window_t win;
	date today;
	ShareData_t tShare;
	ShareData_t tShare2;
	long record = 0;
	long serial = 0;
	int done;

	CLEAR_VAR(tShare);
	category[0] = EOS;

	getdate(&today);
	tShare.Folio = JoinLong(today.da_year, serial);
	tShare.Certificate = record;

	win.SetCursorType(_NOCURSOR);
	win.SetColors(WHITE, BLUE);
	win.SetCaption("Add Share Register Records");
	win.Draw();
	win.Clear();

	DrawShareRegisterFields(win, tShare);

	do {
		do {
			win.Input(tShare.Name1, 40, 29, 6, WHITE, GREEN);
			DrawShareRegisterFields(win, tShare);
		} while (EMPTY_STRING(tShare.Name1));

		win.Input(tShare.Name2, 40, 29, 8, WHITE, GREEN);
		DrawShareRegisterFields(win, tShare);
		win.Input(tShare.Name3, 40, 29, 10, WHITE, GREEN);
		DrawShareRegisterFields(win, tShare);

		do {
			win.Input(tShare.Address, 40, 29, 12, WHITE, GREEN);
			DrawShareRegisterFields(win, tShare);
		} while (EMPTY_STRING(tShare.Address));

		do {
			win.Input(tShare.City, 20, 29, 14, WHITE, GREEN);
			DrawShareRegisterFields(win, tShare);
		} while (EMPTY_STRING(tShare.City));

		do {
			win.Input(&(tShare.PinCode), 29, 16, WHITE, GREEN);
			DrawShareRegisterFields(win, tShare);
		} while (tShare.PinCode < 1);

		do {
			win.Input(tShare.State, 20, 29, 18, WHITE, GREEN);
			DrawShareRegisterFields(win, tShare);
		} while (EMPTY_STRING(tShare.State));

		do {
			win.Input(category, 3, 29, 20, WHITE, GREEN);
			tShare.Category = TranslateCategoryString(category);
			DrawShareRegisterFields(win, tShare);
		} while (tShare.Category == -1);

		do {
			win.Input(&(tShare.Shares), 29, 22, WHITE, GREEN);
			DrawShareRegisterFields(win, tShare);
		} while (tShare.Shares < 1);

		// Invent a unique folio number
		done = FALSE;
		while (!done) {
			tShare.Folio = JoinLong(today.da_year, serial++);

			record = 0;
			done = TRUE;
			while (ShareRegister.Get(record, &tShare2)) {
				if (tShare.Folio == tShare2.Folio) {
					done = FALSE;
				}
				record++;
			}
		}

		// See if a matching folio no exist
		record = 0;
		while (ShareRegister.Get(record, &tShare2)) {
			if (stricmp(tShare.Name1, tShare2.Name1) == EQUAL && stricmp(tShare.Name2, tShare2.Name2) == EQUAL && stricmp(tShare.Name3, tShare2.Name3) == EQUAL && tShare.Category == tShare2.Category) {
				tShare.Folio = tShare2.Folio;
				break;
			}
			record++;
		}

		// Invent a new certificate number
		record = 0;
		while (ShareRegister.Get(record, &tShare2)) record++;
		tShare.Certificate = record;

		DrawShareRegisterFields(win, tShare);

		ShareRegister.Add(&tShare);
	} while (win.Choice("Add another record", "yn", 3, 23, LIGHTRED, BLUE) == 'y');
}

void Application_t::ShareRegisterView() {
	Window_t win;
	Keyboard_t kb;
	ShareData_t tShare;
	long record = 0, recMax = 0;
	int inp = kbNoKey;

	win.SetCursorType(_NOCURSOR);
	win.SetColors(WHITE, BLUE);
	win.SetCaption("View Share Register Records");
	win.Draw();
	win.Clear();
	win.SetColors(LIGHTRED + BLINK, BLUE);
	win.GoToXY(3, 23);
	win.PutS("Use the cursor control keys to change, ESC to return to menu...");

	// Find out total number of records
	while (ShareRegister.Get(recMax, &tShare)) recMax++;
	recMax--;

	if (recMax < 0) return;	// no records to view!

	Assert(ShareRegister.Get(record, &tShare));	// read first record

	while (inp != kbEsc) {
		DrawShareRegisterFields(win, tShare);

		kb.WaitPress();
		inp = kb.Input();

		switch (inp) {
			case kbDown:
			case kbRight:	// Right key
				record++;	// increment
				break;
			case kbUp:
			case kbLeft:	// Left key
				record--;	// decrement
				break;
			case kbPageDown:
				record += 5;
				break;
			case kbPageUp:
				record -= 5;
				break;
			case kbHome:
				record = 0;
				break;
			case kbEnd:
				record = recMax;
				break;
			case kbEsc:
				break;
			default:
				Beep();
		}

		// Sanity check; check if such a record is present
		if (record > recMax) record = 0;	// wrap
		if (record < 0) record = recMax;	// wrap

		Assert(ShareRegister.Get(record, &tShare));
	}
}

void Application_t::ShareRegisterErase() {
	Window_t win;
	Keyboard_t kb;
	ShareData_t tShare;
	long certificate = 0, rec;

	win.SetCursorType(_NOCURSOR);
	win.SetColors(WHITE, BLUE);
	win.SetCaption("Delete Share Register Records");
	win.Draw();

	do {
		/* Draw an input box */
		win.Clear();
		win.SetBounds(20, 11, 60, 17);
		win.SetColors(YELLOW, BLACK);
		win.SetCaption("Enter Certificate Number");
		win.Draw();
		win.Clear();
		win.Input(&certificate, 15, 2, WHITE, GREEN);

		if ((rec = ShareRegister.Search(certificate)) == -1) {
			Beep();
			win.SetColors(LIGHTRED + BLINK, BLACK);
			win.GoToXY(3, 4);
			win.PutS("Record not found! Press ANY key...");
			kb.Clear();
			kb.WaitPress();
			kb.Clear();
			return;
		}

		Assert(ShareRegister.Get(rec, &tShare));

		win.SetBounds(1, 1, SCREEN_WIDTH, SCREEN_HEIGHT);
		win.SetFocus();
		win.SetColors(WHITE, BLUE);
		win.Clear();

		DrawShareRegisterFields(win, tShare);

		if (win.Choice("Delete this record", "yn", 3, 23, LIGHTRED, BLUE) == 'y') {
			ShareRegister.Erase(rec);
		}
	} while (win.Choice("Delete another record", "yn", 3, 23, LIGHTRED, BLUE) == 'y');
}

void Application_t::DrawTransferSharesFields(Window_t &win, TransferData_t &data) {
	char temp[CHAR_MAX];

	// Draw input fields
	win.SetColors(LIGHTMAGENTA, BLACK);
	win.GoToXY(26, 1);
	win.PrintF("Date: %i/%i/%i", (int)data.Date.da_day, (int)data.Date.da_mon, data.Date.da_year);
	win.SetColors(YELLOW, BLACK);
	win.GoToXY(3, 3);
	win.PrintF("      Serial number: T%ld", data.Serial);
	win.GoToXY(3, 5);
	win.PutS(" Certificate number:");
	win.GoToXY(3, 7);
	win.PrintF("Seller folio number: S%ld", data.SellerFolio);
	win.GoToXY(3, 9);
	win.PutS(" Buyer Folio number: S");

	win.InputDrawField(ltoa(data.Certificate, temp, 10), 11, 24, 5, WHITE, GREEN);
	win.InputDrawField(ltoa(data.BuyerFolio, temp, 10), 10, 25, 9, WHITE, GREEN);
}

void Application_t::TransferShares() {
	Window_t win;
	Keyboard_t kb;
	ShareData_t tShare, tShare2;
	TransferData_t data, tData;
	date today;
	long rec = 0;
	int folioFound;

	CLEAR_VAR(data);

	// Invent a new serial number
	while (TransferRegister.Get(rec, &tData)) rec++;
	data.Serial = rec;

	getdate(&today);
	memcpy(&(data.Date), &today, sizeof(date));

	win.SetCursorType(_NOCURSOR);
	win.SetColors(WHITE, BLUE);
	win.SetCaption("Transfer Shares");
	win.Draw();
	win.Clear();
	win.SetBounds(18, 6, 62, 19);
	win.SetCaption("Enter Data");
	win.SetColors(YELLOW, BLACK);
	win.Draw();
	win.Clear();

	DrawTransferSharesFields(win, data);

	do {
		do {
			win.Input(&(data.Certificate), 24, 5, WHITE, GREEN);
			DrawTransferSharesFields(win, data);
		} while (data.Certificate < 0);

		if ((rec = ShareRegister.Search(data.Certificate)) == -1) {
			Beep();
			win.SetColors(LIGHTRED + BLINK, BLACK);
			win.GoToXY(3, 11);
			win.PutS("Record not found! Press ANY key...");
			kb.Clear();
			kb.WaitPress();
			kb.Clear();
			return;
		}

		Assert(ShareRegister.Get(rec, &tShare));

		data.SellerFolio = tShare.Folio;
		DrawTransferSharesFields(win, data);

		do {
			win.Input(&(data.BuyerFolio), 25, 9, WHITE, GREEN);
			DrawTransferSharesFields(win, data);
		} while (data.BuyerFolio < 0);

		// Verify buyer folio
		rec = 0;
		folioFound = FALSE;
		while (ShareRegister.Get(rec, &tShare2)) {
			if (data.BuyerFolio == tShare2.Folio) {
				folioFound = TRUE;
				break;
			}
			rec++;
		}

		// Sanity check
		if (!folioFound || data.BuyerFolio == data.SellerFolio) {
			Beep();
			win.SetColors(LIGHTRED + BLINK, BLACK);
			win.GoToXY(3, 11);
			win.PutS("Invalid folio number! Press ANY key...");
			kb.Clear();
			kb.WaitPress();
			kb.Clear();
			return;
		}

		if (win.Choice("Sell these shares", "yn", 3, 11, LIGHTRED, BLACK) == 'y') {
			// Get seller record
			Assert((rec = ShareRegister.Search(data.Certificate)) != -1);
			Assert(ShareRegister.Get(rec, &tShare));

			// Prepare new buyer certificate
			tShare2.Certificate = data.Certificate;	// or tShare.Certificate
			data.Shares = tShare2.Shares = tShare.Shares;

			// Delete seller record
			ShareRegister.Erase(rec);

			// Add buyer record
			ShareRegister.Add(&tShare2);

			// Store transaction record
			TransferRegister.Add(&data);
		}
	} while (win.Choice("Transfer more shares", "yn", 3, 11, LIGHTRED, BLACK) == 'y');
}

void Application_t::GenerateReport() {
	Window_t win;
	Keyboard_t kb;
	date today;
	ShareData_t tShare1, tShare2;
	TransferData_t tData;
	long rec1 = 0, rec2;
	long totShares = 0;
	long maxShareFolio = 0;
	long maxTransfers = 0;
	long largestShareHolder = -1;
	long largestTransferRec = -1;
	long riShares = 0;
	long nriShares = 0;
	long dcShares = 0;
	long nrcShares = 0;

	getdate(&today);

	win.SetCursorType(_NOCURSOR);
	win.SetColors(WHITE, BLUE);
	win.SetCaption("Today's Report/Analysis");
	win.Draw();
	win.Clear();

	// Draw date
	win.SetColors(LIGHTMAGENTA, BLUE);
	win.GoToXY(61, 1);
	win.PrintF("Date: %i/%i/%4i", (int)today.da_day, (int)today.da_mon, today.da_year);

	win.GoToXY(3, 23);
	win.PutS("Please wait; processing...");

	// Abort if db is empty
	if (!ShareRegister.Get(rec1, &tShare1)) return;

	// Process all databases and build a report
	rec1 = 0;
	while (ShareRegister.Get(rec1, &tShare1)) {
		totShares += tShare1.Shares;
		riShares += (tShare1.Category == CATEGORY_RI ? tShare1.Shares : 0);
		nriShares += (tShare1.Category == CATEGORY_NRI ? tShare1.Shares : 0);
		dcShares += (tShare1.Category == CATEGORY_DC ? tShare1.Shares : 0);
		nrcShares += (tShare1.Category == CATEGORY_NRC ? tShare1.Shares : 0);

		rec2 = 0;
		while (ShareRegister.Get(rec2, &tShare2)) {
			if (tShare1.Folio == tShare2.Folio && rec1 != rec2) {
			  tShare1.Shares += tShare2.Shares;
			}

			if (tShare1.Shares > maxShareFolio) {
				maxShareFolio = tShare1.Shares;
				largestShareHolder = rec1;
			}

			rec2++;
		}

		rec1++;
	}

	rec1 = rec2 = 0;
	while (TransferRegister.Get(rec1, &tData)) {
		if (memcmp(&today, &(tData.Date), sizeof(date)) == EQUAL) {
			if (tData.Shares > rec2) {
				rec2 = tData.Shares;
				largestTransferRec = rec1;
			}
			maxTransfers++;
		}

		rec1++;
	}

	CLEAR_VAR(tShare1);
	CLEAR_VAR(tData);

	// Get all data
	if (largestShareHolder != -1) {
		Assert(ShareRegister.Get(largestShareHolder, &tShare1));
	}

	if (largestTransferRec != -1) {
		Assert(TransferRegister.Get(largestTransferRec, &tData));
	}

	win.SetColors(YELLOW, BLUE);
	win.GoToXY(3, 4);
	win.PrintF("        Total shares issued: %ld", totShares);
	win.GoToXY(3, 6);
	win.PrintF("Maximum shares share-holder: S%ld (%s, %s and %s)", tShare1.Folio, tShare1.Name1, tShare1.Name2, tShare1.Name3);
	win.GoToXY(3, 8);
	win.PrintF("Shares held by share-holder: %ld (%ld%%)", maxShareFolio, maxShareFolio * 100L / totShares);
	win.GoToXY(3, 10);
	win.PrintF("         Shares held by RIs: %ld (%ld%%)", riShares, riShares * 100L / totShares);
	win.GoToXY(3, 12);
	win.PrintF("        Shares held by NRIs: %ld (%ld%%)", nriShares, nriShares * 100L / totShares);
	win.GoToXY(3, 14);
	win.PrintF("         Shares held by DCs: %ld (%ld%%)", dcShares, dcShares * 100L / totShares);
	win.GoToXY(3, 16);
	win.PrintF("        Shares held by NRCs: %ld (%ld%%)", nrcShares, nrcShares * 100L / totShares);
	win.GoToXY(3, 18);
	win.PrintF("    Today's total transfers: %ld", maxTransfers);
	win.GoToXY(3, 20);
	win.PrintF("   Today's largest transfer: %ld shares (S%ld to S%ld)", tData.Shares, tData.SellerFolio, tData.BuyerFolio);

	win.SetColors(LIGHTRED + BLINK, BLUE);
	win.GoToXY(3, 23);
	win.PutS("Done; press ANY key to return to menu...");

	kb.Clear();
	kb.WaitPress();
	kb.Clear();
}

// How small can yer main() be? ;)
int main() {
	Application_t Application;

	Application.Run();

	return EXIT_SUCCESS;
}

/********************************* The End! *********************************/
