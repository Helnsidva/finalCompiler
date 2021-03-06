#ifndef _SCANNER_H
#define _SCANNER_H

#include "parser.h"

#define MAXINTLENGTH 10 //максимальная длина int
#define MAXINT 2147483647 //максимальное значение int

//терминальные символы
#define nullLexical 0
#define timesLexical 1
#define divLexical 3
#define modLexical 4
#define andLexical 5
#define plusLexical 6
#define minusLexical 7
#define orLexical 8
#define eqlLexical 9
#define neqLexical 10
#define lssLexical 11
#define geqLexical 12
#define leqLexical 13
#define gtrLexical 14
#define periodLexical 18
#define commaLexical 19
#define colonLexical 20
#define rparenLexical 22
#define rbrakLexical 23
#define ofLexical 25
#define thenLexical 26
#define doLexical 27
#define lparenLexical 29
#define lbrakLexical 30
#define notLexical 32
#define becomesLexical 33
#define numberLexical 34
#define identLexical 37
#define semicolonLexical 38
#define endLexical 40
#define elseLexical 41
#define elsifLexical 42
#define ifLexical 44
#define whileLexical 46
#define arrayLexical 54
#define recordLexical 55
#define constLexical 57
#define typeLexical 58
#define varLexical 59
#define procedureLexical 60
#define beginLexical 61
#define moduleLexical 63
#define eofLexical 64

void get(struct parameters*);
void mark(char[], struct parameters*);

#endif