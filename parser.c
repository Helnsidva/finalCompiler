#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "scanner.h"

void EnterKW(int sym, char name[], struct keyLex* keyTab[], int* i) {

    keyTab[*i]->sym = sym;
    strcpy(keyTab[*i]->id, name);
    (*i)++;

}

void InitLexical(struct parameters* storage) {

    for(int i = 0; i < 34; i++) {
        storage->keyTab[i] = (struct keyLex*)malloc(sizeof(struct keyLex));
    }

    int i = 0;
    EnterKW(nullLexical, "BY", storage->keyTab, &i);
    EnterKW(doLexical, "DO", storage->keyTab, &i);
    EnterKW(ifLexical, "IF", storage->keyTab, &i);
    EnterKW(nullLexical, "IN", storage->keyTab, &i);
    EnterKW(nullLexical, "IS", storage->keyTab, &i);
    EnterKW(ofLexical, "OF", storage->keyTab, &i);
    EnterKW(orLexical, "OR", storage->keyTab, &i);
    EnterKW(nullLexical, "TO", storage->keyTab, &i);
    EnterKW(endLexical, "END", storage->keyTab, &i);
    EnterKW(nullLexical, "FOR", storage->keyTab, &i);
    EnterKW(modLexical, "MOD", storage->keyTab, &i);
    EnterKW(nullLexical, "NIL", storage->keyTab, &i);
    EnterKW(varLexical, "VAR", storage->keyTab, &i);
    EnterKW(nullLexical, "CASE", storage->keyTab, &i);
    EnterKW(elseLexical, "ELSE", storage->keyTab, &i);
    EnterKW(nullLexical, "EXIT", storage->keyTab, &i);
    EnterKW(thenLexical, "THEN", storage->keyTab, &i);
    EnterKW(typeLexical, "TYPE", storage->keyTab, &i);
    EnterKW(nullLexical, "WITH", storage->keyTab, &i);
    EnterKW(arrayLexical, "ARRAY", storage->keyTab, &i);
    EnterKW(beginLexical, "BEGIN", storage->keyTab, &i);
    EnterKW(constLexical, "CONST", storage->keyTab, &i);
    EnterKW(elsifLexical, "ELSIF", storage->keyTab, &i);
    EnterKW(nullLexical, "IMPORT", storage->keyTab, &i);
    EnterKW(nullLexical, "UNTIL", storage->keyTab, &i);
    EnterKW(whileLexical, "WHILE", storage->keyTab, &i);
    EnterKW(recordLexical, "RECORD", storage->keyTab, &i);
    EnterKW(nullLexical, "REPEAT", storage->keyTab, &i);
    EnterKW(nullLexical, "RETURN", storage->keyTab, &i);
    EnterKW(nullLexical, "POINTER", storage->keyTab, &i);
    EnterKW(procedureLexical, "PROCEDURE", storage->keyTab, &i);
    EnterKW(divLexical, "DIV", storage->keyTab, &i);
    EnterKW(nullLexical, "LOOP", storage->keyTab, &i);
    EnterKW(moduleLexical, "MODULE", storage->keyTab, &i);

}

int init(struct parameters* storage, char* sourceCode) {

    //буду сюда писать все по мере необходимости!
    storage->sourceCode = sourceCode;
    storage->lastPosition = 0;
    strcpy(storage->lastLexeme, "\0");
    storage->lastLexemeCode = -1;
    storage->lastLexemeValue = -1;
    storage->error = 0;
    storage->errpos = -1;

    storage->reportFile = NULL;
    if((storage->reportFile = fopen("report.txt", "wb")) == NULL) {
        printf("Opening report.txt file error!\n");
        return -1;
    }

    InitLexical(storage);

    storage->linesCounter = 0;

    return 0;

}

void module(struct parameters* storage) {


}

char* Compile(char* sourceCode) {

    struct parameters* storage =
            (struct parameters*)malloc(sizeof(struct parameters));

    if(init(storage, sourceCode) != 0) {
        return NULL;
    }

    get(storage);

    module(storage); //todo 0

    //return storage->outputCode;
    return storage->sourceCode;

}