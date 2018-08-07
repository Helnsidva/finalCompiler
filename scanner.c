#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "scanner.h"

#define MAXINTLENGTH 10
#define MAXINT 2147483647

void Mark(char msg[], struct parameters* storage) {

    int p;
    p = storage->lastPosition;
    if (p > storage->errpos) {
        fprintf(storage->reportFile, "error: position %d %s \n", p, msg);
        printf("error: position %d %s \n", p, msg);
    }
    storage->errpos = p;
    storage->error = 1;

}

void comment(struct parameters* storage) {

    int index = storage->lastPosition + 2;
    for (index; storage->sourceCode[index] != '\0'; index++) {
        if (storage->sourceCode[index] == ')') {
            if (storage->sourceCode[index - 1] == '*') {
                break;
            }
        }
    }
    if (storage->sourceCode[index] == '\0') {
        Mark("comment is not completed", storage);
    }
    else {
        index++;
    }
    storage->lastPosition = index;

}

void Number(struct parameters* storage) {

    int index = storage->lastPosition;
    int indexString = 0;
    long long int getNumber = 0;
    char numberString[MAXINTLENGTH + 1]; //string for recording number

    for (indexString, index; (storage->sourceCode[index] != '\0') &&
                             isdigit(storage->sourceCode[index]) && (indexString < MAXINTLENGTH); indexString++, index++) {
        numberString[indexString] = storage->sourceCode[index];
        numberString[indexString + 1] = '\0';
    }

    if (isdigit(storage->sourceCode[index])) { //if numb length is too big
        while ((storage->sourceCode[index] != '\0') &&
               (isdigit(storage->sourceCode[index]))) index++; //go to the end of numb
        Mark("too big number", storage);
    }
    else {
        getNumber = atoll(numberString);
        if (getNumber > MAXINT) {
            Mark("too big number", storage);
        }
        else {
            storage->lastLexemeCode = numberLexical;
            storage->lastLexemeValue = getNumber;
        }
    }
    storage->lastPosition = index;

}

void Ident(struct parameters* storage) {

    int index = storage->lastPosition;
    int indexString = 0;
    char identification[idLen + 1];

    for (indexString, index; (storage->sourceCode[index] != '\0') &&
                             (isalnum(storage->sourceCode[index])) && (indexString < idLen); indexString++, index++) {
        identification[indexString] = storage->sourceCode[index];
    }
    identification[indexString] = '\0';

    if (isalnum(storage->sourceCode[index])) {
        while ((storage->sourceCode[index] != '\0') && (isalnum(storage->sourceCode[index]))) index++;
        Mark("too large identification", storage);
    }
    else {
        int k = 0;
        while((k < kw) && (strcmp(identification, storage->keyTab[k]->id))) k++;
        if (k < kw) {
            storage->lastLexemeCode = storage->keyTab[k]->sym;
        }
        else {
            storage->lastLexemeCode = identLexical;
            strcpy(storage->lastLexeme, identification);
        }
    }
    storage->lastPosition = index;

}

void get(struct parameters* storage) {

    char readChar;
    int index = storage->lastPosition;

    storage->lastLexemeCode = -1;
    strcpy(storage->lastLexeme, "\0");
    storage->lastLexemeValue = -1;

    while ((storage->sourceCode[index] != '\0') && (storage->sourceCode[index] <= ' ')) index++;
    readChar = storage->sourceCode[index];

    storage->lastPosition = index;

    if (readChar == '\0') {
        storage->lastLexemeCode = eofLexical;
    }
    else {
        switch (readChar) {
            case '&':
                storage->lastLexemeCode = andLexical;
                storage->lastPosition++;
                break;
            case '*':
                storage->lastLexemeCode = timesLexical;
                storage->lastPosition++;
                break;
            case '+':
                storage->lastLexemeCode = plusLexical;
                storage->lastPosition++;
                break;
            case '-':
                storage->lastLexemeCode = minusLexical;
                storage->lastPosition++;
                break;
            case '=':
                storage->lastLexemeCode = eqlLexical;
                storage->lastPosition++;
                break;
            case '#':
                storage->lastLexemeCode = neqLexical;
                storage->lastPosition++;
                break;
            case '<':
                if (storage->sourceCode[index + 1] == '=') {
                    storage->lastLexemeCode = leqLexical;
                    storage->lastPosition += 2;
                }
                else {
                    storage->lastLexemeCode = lssLexical;
                    storage->lastPosition++;
                }
                break;
            case '>':
                if (storage->sourceCode[index + 1] == '=') {
                    storage->lastLexemeCode = geqLexical;
                    storage->lastPosition += 2;
                }
                else {
                    storage->lastLexemeCode = gtrLexical;
                    storage->lastPosition++;
                }
                break;
            case ';':
                storage->lastLexemeCode = semicolonLexical;
                storage->lastPosition++;
                break;
            case ',':
                storage->lastLexemeCode = commaLexical;
                storage->lastPosition++;
                break;
            case ':':
                if (storage->sourceCode[index + 1] == '=') {
                    storage->lastLexemeCode = becomesLexical;
                    index += 2;
                }
                else {
                    storage->lastLexemeCode = colonLexical;
                    storage->lastPosition++;
                }
                break;
            case '.':
                storage->lastLexemeCode = periodLexical;
                storage->lastPosition++;
                break;
            case '(':
                if (storage->sourceCode[index + 1] == '*') {
                    comment(storage);
                    get(storage);
                }
                else {
                    storage->lastLexemeCode = lparenLexical;
                    storage->lastPosition++;
                }
                break;
            case ')':
                storage->lastLexemeCode = rparenLexical;
                storage->lastPosition++;
                break;
            case '[':
                storage->lastLexemeCode = lbrakLexical;
                storage->lastPosition++;
                break;
            case ']':
                storage->lastLexemeCode = rbrakLexical;
                storage->lastPosition++;
                break;
            case '~':
                storage->lastLexemeCode = notLexical;
                break;
            default:
                if (isdigit(readChar)) {
                    Number(storage);
                }
                else if (isalpha(readChar)) {
                    Ident(storage);
                }
                else {
                    storage->lastLexemeCode = nullLexical;
                    storage->lastPosition++;
                    Mark("unknown symbol", storage); //todo
                }
                break;
        }
    }

}