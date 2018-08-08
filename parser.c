#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "scanner.h"
#include "codeGenerator.h"

void signal(char msg[], struct parameters* storage) {
    fprintf(storage->reportFile, "%s\r\n", msg);
    printf("%s\n", msg);
}

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
    storage->outputCode = NULL;
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

    storage->curlev = 0;
    storage->pc = 0;
    storage->cno = 0;
    memset(storage->regs, 0, sizeof(int) * 16);

    return 0;

}



void openScope() {
    printf("openScope\n");
}

void declarations() {
    printf("declarations\n");
}

void ProcedureDecl() {
    printf("ProcedureDecl\n");
}

void headerGenerator() {
    printf("headerGenerator\n");
}

void StatSequence() {
    printf("StatSequence\n");
}

void CloseScope() {
    printf("CloseScope\n");
}

void closeGenerator() {
    printf("closeGenerator\n");
}

void module(struct parameters* storage) {

    char modid[idLen] = "\0"; //название модуля
    int varsize; //??

    signal("Compilation begins.", storage);

    if(storage->lastLexemeCode == moduleLexical) {

        get(storage);
        openScope(); //todo
        varsize = 0;

        if(storage->lastLexemeCode == identLexical) {
            strcpy(modid, storage->lastLexeme);
            char recordingString[idLen + 16] = "Compiles module ";
            strcat(recordingString, modid);
            strcat(recordingString, ".");
            signal(recordingString, storage);
            get(storage);
        }
        else {
            Mark("module ident?", storage);
        }

        if(storage->lastLexemeCode == semicolonLexical)
            get(storage);
        else
            Mark(";?", storage);

        declarations(); //todo

        while(storage->lastLexemeCode == procedureLexical) {

            ProcedureDecl(); //todo

            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                Mark(";?", storage);
        }

        headerGenerator(); //todo

        if(storage->lastLexemeCode == beginLexical) {
            get(storage);
            StatSequence(); //todo
        }

        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            Mark("END?", storage);
        if(storage->lastLexemeCode == identLexical) {
            if(strcmp(modid, storage->lastLexeme)) {
                Mark("wrong module ident", storage);
            }
            get(storage);
        }
        else
            Mark("module ident?", storage);
        if(storage->lastLexemeCode != periodLexical)
            Mark(".?", storage);

        CloseScope(); //todo

        if(!storage->error) {

            closeGenerator(); //todo
            signal("Code generated.", storage);

        }

    }
    else {
        Mark("module?", storage);
    }

}

char* Compile(char* sourceCode) {

    struct parameters* storage =
            (struct parameters*)malloc(sizeof(struct parameters));

    if(init(storage, sourceCode) != 0) {
        return NULL;
    }

    get(storage);

    module(storage); //todo 0

    signal("Compilation finished.", storage);

    return storage->outputCode;

}