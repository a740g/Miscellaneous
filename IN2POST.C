/*
Infix expression to postfix converter.
And, postfix expression evaluator.
Limitations:
	The string parser is very very simple. :)
	Operands are only a character long.
	No negative operands.
	The infix expression is not checked for validity.
	Does not like characters like spaces, tabs etc. Avoid them!

Samuel Gomes, 2003, Jul, 1
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define debug_print(_msg_) fprintf(stderr, "%s\n", (_msg_))
#define STACK_SIZE 4096

typedef struct {
	int top;
	double items[STACK_SIZE];
} Stack_t;

void Stack_Initialize(Stack_t *stk) {
	stk->top = -1;
}

int Stack_Empty(Stack_t *stk) {
	return (stk->top < 0);
}

int Stack_Full(Stack_t *stk) {
	return (stk->top >= STACK_SIZE);
}

void Stack_Push(Stack_t *stk, double item) {
	if (Stack_Full(stk)) return;

	stk->items[++(stk->top)] = item;
}

double Stack_Pop(Stack_t *stk) {
	if (Stack_Empty(stk)) return 0;

	return stk->items[stk->top--];
}

double Stack_Top(Stack_t *stk) {
	if (Stack_Empty(stk)) return 0;

	return stk->items[stk->top];
}

void pause(void) {
	puts("Press any key to continue...");
	getchar();
}

/* operator rankings */
int oprank(char op) {
	switch (op) {
		case '^':
			return 3;
		case '*':
		case '%':
		case '/':
			return 2;
		case '+':
		case '-':
			return 1;
	}

	debug_print("Illegal operator encountered!");
	return 0; /* what the hell! */
}

/* does op1 has precedence over op2? */
int precedence(char op1, char op2) {
	/* special parenthesis case (yuck!) */
	if (op2 == '(' && op1 != ')') return 0;
	if (op1 == '(') return 0;
	if (op1 != '('&& op2 == ')') return 1;
	if (op1 == '(' && op2 == ')') return 0;
	/* special power case */
	if (op1 == op2 && op1 == '^') return 0;
	/* normal cases */
	return (oprank(op1) >= oprank(op2));
}

/* convert infix to postfix */
void postfix(char infix[], char postr[]) {
	int inpos, outpos = 0;
	char symb, topsymb;
	Stack_t opstk;

	Stack_Initialize(&opstk);

	for (inpos = 0; infix[inpos] != '\0'; inpos++) {
		symb = infix[inpos];

		/* If symbol is a alphabet or a number */
		if (isalnum(symb)) {
			postr[outpos++] = symb;
		}
		else {
			while (!Stack_Empty(&opstk) && precedence((char)Stack_Top(&opstk), symb)) {
				topsymb = Stack_Pop(&opstk);
				postr[outpos++] = topsymb;
			}

			/* again a stupid paranthesis hack */
			if (Stack_Empty(&opstk) || symb != ')') {
				Stack_Push(&opstk, symb);
			}
			else {
				topsymb = Stack_Pop(&opstk);
			}
		}
	}

	while (!Stack_Empty(&opstk)) {
		postr[outpos++] = Stack_Pop(&opstk);
	}

	postr[outpos] = '\0';
}

/* executes an operation */
double oper(char op, double opnd1, double opnd2) {
	switch (op) {
		case '^':
			return pow(opnd1, opnd2);
		case '*':
			return (opnd1 * opnd2);
		case '%':
			return fmod(opnd1, opnd2);
		case '/':
			return (opnd1 / opnd2);
		case '+':
			return (opnd1 + opnd2);
		case '-':
			return (opnd1 - opnd2);
	}

	debug_print("Illegal operator encountered!");
	return 0;	/* oh no! */
}

/* evaluates a postfix expression */
double eval(char expr[]) {
	int pos;
	char c;
	double opnd1, opnd2, value;
	Stack_t opndstk;

	Stack_Initialize(&opndstk);

	for (pos = 0; expr[pos] != '\0'; pos++) {
		c = expr[pos];

		if (isalnum(c)) {
			if (isdigit(c)) {
				Stack_Push(&opndstk, c - '0');
			}
			else {
				Stack_Push(&opndstk, 0);	/* jero! */
			}
		}
		else {
			opnd2 = Stack_Pop(&opndstk);
			opnd1 = Stack_Pop(&opndstk);
			value = oper(c, opnd1, opnd2);
			Stack_Push(&opndstk, value);
		}
	}

	return Stack_Pop(&opndstk);		/* yeeha! */
}

void main() {
	char infix[80 * 25];
	char postr[80 * 25];

	puts("enter a *valid* infix expression:");
	gets(infix);

	printf("infix expression: %s\n", infix);
	postfix(infix, postr);
	printf("postfix expression: %s\n", postr);
	printf("if your infix expression was correct "
		"and contained only digits, then here is "
		"the answer:\n%g\n", eval(postr));

	pause();
}
