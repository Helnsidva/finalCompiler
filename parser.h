#ifndef _PARSER_H
#define _PARSER_H

#define identLength 16 //максимальная длина идентификатора
#define keyTabSize 34 //количество терминальных символов языка
#define maxCodeSize 1000 //максимальная длина кода
#define wordSize 4 //размер одного слова

struct symbolLex { //структура для соответствия терминального символа и его кода

    int symbol;
    char identifier[12];

};

struct object { //структура для представления одного объекта

    int class; //класс объекта: переменная, функция, константа, параметр, поле, тип, ...
    int level; //уровень вложенности своего поля видимости
    struct object* nextObject; //указатель на следующий элемент в своем поле
    struct object* previousScope; //указатель на предыдущее поле
    struct type* classType; //тип класса (например, int или bool)
    char name[identLength]; //имя объекта
    int value; //значение/сдвиг относительно базового адреса

};

struct type { //структура для предствления типа

    int classType; //bool, int, массив, либо запись
    struct object* fields; //поля записи
    struct type* baseType; //тип массива
    int size; //размер
    int length; //длина(массива)

};

struct item { //структура для представления объекта с учетом его синтаксиса

    int mode; //режим выражения. var, const, reg
    int level; //уровень вложенности своего поля видимости
    struct type* classType; //тип
    int a; //первый аргумент команды
    int b; //второй аргумент команды
    int c; //третий аргумент команды
    int storage; //регистр

};

struct parameters {

    char* sourceCode; //исходный код
    int lastPosition; //последняя просмотренная позиция исходного кода
    char lastLexeme[identLength]; //последний считанный идентификатор
    int lastLexemeCode; //код считанного последнего символа
    int lastLexemeValue; //последнее считанное число
    int error; //флаг ошибки
    int errorPosition; //флаг, указывающий на последнюю позицию ошибки
    FILE* reportFile; //файл с информацией о компиляции
    struct symbolLex* keyTab[keyTabSize]; //структура для сканера. хранит множество терминальных символов
    int currentLevel; //текущий уровень вложенности
    int PC; //счетчик команд
    int registers[16]; //множество регистров
    struct type* boolType; //глобальный тип bool
    struct type* intType; //глобальный тип int
    struct object* topScope;
    struct object* guard;
    struct object* universe; //глобальные области видимости
    int code[maxCodeSize]; //скомпилированный код
    int entryAddress; //адрес входа в программу
    int linesCounter; //счетчик строк
    int posCounter; //счетчик позиции в строке
    struct item* emptyItem; //пустой item для возврата при ошибках памяти
    struct object* emptyObject; //пустой object для возврата при ошибках памяти
    struct type* emptyType; //пустой type для возврата при ошибках памяти

};

void compile(char*);
void initItem(struct item *, struct parameters *);

#endif