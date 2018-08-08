#define idLen 16 //max length of identifier
#define kw 34 //number of term symbols

struct keyLex {

    int sym;
    char id[12];

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

};

char* Compile(char* );