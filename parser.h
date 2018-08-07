#define idLen 16 //max length of identifier
#define kw 34 //number of term symbols

struct keyLex {

    int sym;
    char id[12];

};

struct parameters {

    char* sourceCode;
    char* outputCode;

    int lastPosition;

    char lastLexeme[idLen];
    int lastLexemeCode;
    int lastLexemeValue;

    int error;
    int errpos;

    FILE* reportFile;

    struct keyLex* keyTab[kw];

};

char* Compile(char* );