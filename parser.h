#define idLen 16 //max length of identifier
#define kw 34 //number of term symbols
#define maxCode 1000
#define WordSize 4
#define NofCom 16

struct keyLex {

    int sym;
    char id[12];

};

struct Object {
    int class; //переменная, функция, ..
    int lev; //уровень
    struct Object* next; //следующий
    struct Object* dsc; //предыдущий объект при открытии scope
    struct Type* type; //тип переменной, ..
    char name[16]; //16 - max identifier length
    int val; //значение для числа
};

struct Type {
    int form;
    struct Object* fields;
    struct Type* base;
    int size;
    int len;
};

struct Item {
    int mode;
    int lev;
    struct Type* type;
    int a;
    int b;
    int c;
    int r;
};

struct parameters {

    char* sourceCode; //исходный код
    char* outputCode;

    int lastPosition; //последняя просмотренная позиция

    char lastLexeme[idLen]; //последний идентификатор
    int lastLexemeCode; //код последнего символа
    int lastLexemeValue; //последнее число

    int error; //флаг ошибки
    int errpos; //зачем?

    FILE* reportFile; //файл с ошибками

    struct keyLex* keyTab[kw]; //для инициализации сканера

    int linesCounter; //количество строк. для индексации (индексы без \r)

    int curlev; //текущий уровень
    int pc; //индекс след. команды ?
    int cno; //???
    int regs[16]; //множество свободных регистров

    struct Type* boolType;
    struct Type* intType;

    struct Object* topScope;
    struct Object* guard;
    struct Object* universe;

    struct Object* lastNewObject;
    struct Object* firstObject;

    int code[maxCode]; //собственно код
    char comname[NofCom][idLen];
    int comadr[NofCom];
    char mnemo[64][5];

};

char* Compile(char* );
