#include <stdio.h>
#include <malloc.h>
#include "parser.h"

char* readCode(char* fileName) {

    //чтение исходного кода из файла
    FILE* sourceFile = NULL;
    int fileSize = 0; //размер входного файла. нужен чтобы знать количество считываемых символов
    char* sourceCode;
    printf("Oberon-0 Compiler 1.0\n");
    printf("Opening source code file...\n");
    if((sourceFile = fopen(fileName, "rb")) == NULL) {
        printf("Opening source file error!\nCompilation terminated.");
        return NULL;
    }
    printf("Reading source code...\n");
    fseek(sourceFile, 0, SEEK_END);
    fileSize = ftell(sourceFile);
    rewind(sourceFile); //возвращаемся в началу файла
    sourceCode = (char*)malloc((fileSize + 1) * sizeof(char));
    if(sourceCode == NULL)
        return NULL;
    fread(sourceCode, sizeof(char), fileSize, sourceFile);
    sourceCode[fileSize] = '\0';
    fclose(sourceFile);
    return sourceCode;

}

void manual() {

    //печать информации о программе и аргументах
    printf("Oberon-0 Compiler 1.0.\n\n");
    printf("Invalid command line format!\n");
    printf("Correct format: \"Oberon-0-Compiler.exe source.txt\", where \"source.txt\" - file with source code in the Oberon-0 language.\n");
    printf("Example of Oberon-0 code: \nMODULE If;\n"
                   "VAR \n"
                   "\t\ti, j, z: INTEGER;\n"
                   "\tBEGIN\n"
                   "\t\ti := 1;\n"
                   "\t\tj := 2;\n"
                   "\t\tIF (i > j) THEN \n"
                   "\t\t\tz := j;\n"
                   "\t\tELSIF (i = j) THEN\n"
                   "\t\t\tz := 5;\n"
                   "\t\tELSE \n"
                   "\t\t\tz := 10;\n"
                   "\t\tEND;\n"
                   "END If.\n");
    printf("Compiler compiles for RISC machine.\n");
    printf("Compiled code is saved in the current directory in \"output.txt\" file.\n");
    return;

}

int main(int argc, char* argv[]) {


    //вид командной строки: compiler.exe имя_входного_файла
    if(argc != 2)
        manual(); //печать информации о программе и аргументах
    else {
        char* inputFileName = NULL; //имя входного файла
        char* sourceCode = NULL; //исходный код
        inputFileName = argv[1];
        sourceCode = readCode(inputFileName); //чтение исходного кода из файла
        if(sourceCode != NULL) {
            compile(sourceCode); //если код считан успешно - начало компиляции
        }
    }
    return 0;

}