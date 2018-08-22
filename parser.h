#define identLength 16 //максимальная длина идентификатора
#define keyTabSize 34 //количество терминальных символов языка
#define maxCodeSize 1000 //максимальная длина кода
#define wordSize 4 //размер одного слова

struct keyLex { //структура для соответствия терминального символа и его кода

    int symbol;
    char identifier[12];

};

struct machineCommand { //структура для одной машинной команды

    int command; //команда
    int a;
    int b;
    int c; //три аргумента

};

struct Object { //структура для представления одного объекта (функция, переменная, константа, параметр, ...)
    int class; //переменная, функция, константа, параметр, поле, тип, ...
    int level; //уровень вложенности своего поля видимости
    struct Object* nextObject; //указатель на следующий элемент в своем поле
    struct Object* previousScope; //указатель на предыдущее поле
    struct Type* type; //тип класс (например, int или bool)
    char name[identLength]; //имя объекта
    int value; //значение/сдвиг относительно базового адреса
};

struct Type { //структура для предствления типа
    int type; //bool, int, массив, либо запись
    struct Object* fields; //поля записи
    struct Type* baseType; //тип массива
    int size; //размер
    int length; //длина (массива)
};

struct Item { //структура для представления объекта с учетом его синтаксиса
    int mode; //режим выражения. var, const, reg
    int level; //уровень вложенности своего поля видимости
    struct Type* type; //тип
    int a; //первый аргумент команды
    int b; //второй аргумент команды
    int c; //третий аргумент команды
    int storage; //регистр
};

struct parameters {
    char* sourceCode; //исходный код
    int lastPosition; //последняя просмотренная позиция исходного кода
    char lastLexeme[identLength]; //последний идентификатор
    int lastLexemeCode; //код последнего символа
    int lastLexemeValue; //последнее число
    int error; //флаг ошибки
    int errorPosition; //флаг, указывающий на последнюю ошибку
    FILE* reportFile; //файл с ошибками
    struct keyLex* keyTab[keyTabSize]; //структура для сканера. хранит множество терминальных символов
    int linesCounter; //количество строк. для индексации (индексы без \r)
    int currentLevel; //текущий уровень вложенности
    int PC; //индекс следующей команды
    int registers[16]; //множество свободных регистров
    struct Type* boolType; //глобальный тип bool
    struct Type* intType; //глобальный тип int
    struct Object* topScope;
    struct Object* guard;
    struct Object* universe; //глобальные области видимости
    struct machineCommand* code[maxCodeSize]; //скомпилированный код
    int entryAddress; //адрес входа? //todo
};

void compile(char *);
