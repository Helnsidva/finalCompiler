#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "scanner.h"

#define MAXINTLENGTH 10 //максимальная длина int
#define MAXINT 2147483647 //максимальное значение int

void mark(char msg[], struct parameters* storage) {

    //печать сообщения об ошибке в report.txt и в консоль
    int p;
    p = storage->lastPosition;
    if (p > storage->errorPosition) {
        fprintf(storage->reportFile, "error: position %d %s\r\n", p - storage->linesCounter, msg);
        printf("error: position %d %s \n", p - storage->linesCounter, msg);
    }
    storage->errorPosition = p;
    storage->error = 1; //обновление индексов ошибки

}

void comment(struct parameters* storage) {

    //пропуск комментария
    int index = storage->lastPosition + 2; //(* comment *). здесь пропускаем (*
    for (index; storage->sourceCode[index] != '\0'; index++) {
        if (storage->sourceCode[index] == '\r')
            storage->linesCounter++;
        if (storage->sourceCode[index] == ')') {
            if (storage->sourceCode[index - 1] == '*')
                break;
        }
    }
    if (storage->sourceCode[index] == '\0')
        mark("Comment is not terminated", storage); //если дошли до конца файла - комментарий не окончен
    else
        index++;
    storage->lastPosition = index;

}

void number(struct parameters *storage) {

    //чтение числа
    int index = storage->lastPosition; //первая позиция числа
    int indexString = 0;
    long long int getNumber = 0; //полученное число
    char numberString[MAXINTLENGTH + 1]; //строка для записи числа
    for (indexString, index; (storage->sourceCode[index] != '\0') &&
                             isdigit(storage->sourceCode[index]) && (indexString < MAXINTLENGTH); indexString++, index++) {
        numberString[indexString] = storage->sourceCode[index];
        numberString[indexString + 1] = '\0';
    } //считываем цифры, пока не конец строки и пока число не длиннее максимально допустимого
    if (isdigit(storage->sourceCode[index])) { //если число длиннее допустимого
        while ((storage->sourceCode[index] != '\0') &&
               (isdigit(storage->sourceCode[index])))
            index++; //идем до конца числа
        mark("Too big number", storage);
        storage->lastLexemeCode = nullLexical;
    }
    else {
        getNumber = atoll(numberString);
        if (getNumber > MAXINT) { //если число превышает максимально допустимое
            mark("Too big number", storage);
            storage->lastLexemeCode = nullLexical;
        }
        else { //если ошибок нет - записываем значение и код
            storage->lastLexemeCode = numberLexical;
            storage->lastLexemeValue = getNumber;
        }
    }
    storage->lastPosition = index;
    if (isalpha(storage->sourceCode[index]))
        mark("No space?", storage); //если закончили на букве, к примеру, 7777IF

}

void identifier(struct parameters *storage) {

    //чтение идентификатора
    int index = storage->lastPosition; //первая позиция идентификатора
    int indexString = 0;
    char identification[identLength + 1]; //строка для идентификатора
    for (indexString, index; (storage->sourceCode[index] != '\0') &&
                             (isalnum(storage->sourceCode[index])) && (indexString < identLength); indexString++, index++)
        identification[indexString] = storage->sourceCode[index];
    identification[indexString] = '\0'; //считываем буквы и цифры, пока не конец файла и до identLength

    if (isalnum(storage->sourceCode[index])) { //если идентификатор длиннее identLength
        while (isalnum(storage->sourceCode[index]))
            index++; //идем до конца идентификатора
        mark("Too large identification", storage);
        storage->lastLexemeCode = nullLexical;
    }
    else {
        int k = 0;
        while((k < keyTabSize) && (strcmp(identification, storage->keyTab[k]->identifier)))
            k++; //ищем идентификатор в таблице терминальных символов
        if(k < keyTabSize)
            storage->lastLexemeCode = storage->keyTab[k]->symbol; //если нашли - присваиваем его код
        else {
            storage->lastLexemeCode = identLexical;
            strcpy(storage->lastLexeme, identification); //если нет - записываем идентификатор и код идентификатора
        }
    }
    storage->lastPosition = index;

}

void get(struct parameters* storage) {

    //получение следующего символа
    char readChar; //текущий символ
    int index = storage->lastPosition; //текущий индекс
    storage->lastLexemeCode = -1;
    strcpy(storage->lastLexeme, "\0");
    storage->lastLexemeValue = -1; //"сбрасываем" все данные о предыдущем символе
    while((storage->sourceCode[index] != '\0') && (storage->sourceCode[index] <= ' ')) {
        if(storage->sourceCode[index] == '\r')
            storage->linesCounter++;
        index++;
    } //пропускаем символы табуляции и пробелы
    readChar = storage->sourceCode[index];
    storage->lastPosition = index;
    if (readChar == '\0')
        storage->lastLexemeCode = eofLexical; //если дошли до символа конца строки - конец файла
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
                    storage->lastPosition += 2; //либо <, либо <=
                }
                else {
                    storage->lastLexemeCode = lssLexical;
                    storage->lastPosition++;
                }
                break;
            case '>':
                if (storage->sourceCode[index + 1] == '=') {
                    storage->lastLexemeCode = geqLexical;
                    storage->lastPosition += 2; //либо >, либо >=
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
                    storage->lastPosition += 2; //либо :, либо :=
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
                    get(storage); //либо комментарий (* ... *), либо 
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
                storage->lastPosition++;
                break;
            default:
                if (isdigit(readChar)) {
                    number(storage);
                }
                else if (isalpha(readChar)) {
                    identifier(storage);
                }
                else {
                    storage->lastLexemeCode = nullLexical;
                    storage->lastPosition++;
                    mark("unknown symbol", storage); //todo
                }
                break;
        }
    }

}