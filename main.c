#include <stdio.h>
#include <malloc.h>
#include "parser.h"

char* readCode(char* fileName) {

    FILE* sourceFile = NULL;
    int fileSize = 0;
    char* sourceCode;
    printf("Oberon-0 Compiler 1.0\n");
    printf("Opening source code file...\n");
    if((sourceFile = fopen("source.txt", "rb")) == NULL) {
        printf("Opening source file error!\nCompilation terminated.");
        return NULL;
    }
    printf("Reading source code...\n");
    fseek(sourceFile, 0, SEEK_END);
    fileSize = ftell(sourceFile);
    rewind(sourceFile); //get file size
    sourceCode = (char*)malloc((fileSize + 1) * sizeof(char));
    fread(sourceCode, sizeof(char), fileSize, sourceFile);
    sourceCode[fileSize] = '\0'; //get source code
    fclose(sourceFile);
    return sourceCode;

}

void manual() {

    printf("Oberon-0 Compiler 1.0.\n\n");
    printf("Invalid command line format!\n");
    printf("Correct format: \"Oberon-0-Compiler.exe source.txt\", where \"source.txt\" - file with source code in the Oberon-0 language.\n");
    printf("Compiler compiles for RISC machine.\n");
    printf("Compiled code is saved in the current directory in \"output.txt\" file.\n");
    return;

}

int main(int argc, char* argv[]) {

    char* sourceCode = NULL;
    char* inputFileName = NULL;
    if(argc != 2) {
        manual();
    }
    else {
        inputFileName = argv[1];
        sourceCode = readCode(inputFileName);
        if(sourceCode != NULL) {
            compile(sourceCode); //начало
        }
    }
    return 0;

}