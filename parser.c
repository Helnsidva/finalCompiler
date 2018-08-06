#include <stdio.h>
#include <malloc.h>
#include "parser.h"

void init(struct parameters* storage, char* sourceCode) {

    //буду сюда писать все по мере необходимости!
    storage->sourceCode = sourceCode;

}

void get() {



}

void module() {



}

char* Compile(char* sourceCode) {

    struct parameters* storage =
            (struct parameters*)malloc(sizeof(struct parameters));

    init(storage, sourceCode);

    get();

    module();

    return storage->outputCode;

}