#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "scanner.h"
#include "codeGenerator.h"

void signal(char msg[], struct parameters* storage) {

    //печать сообщения в report.txt и в консоль
    fprintf(storage->reportFile, "%s\r\n", msg);
    printf("%s\n", msg);

}

void enterUniverse(int class, int value, char *name, struct type *type, struct parameters *storage) {

    //создание объектов в universe
    struct object* newObject = (struct object*)malloc(sizeof(struct object));
    newObject->class = class;
    newObject->value = value;
    strcpy(newObject->name, name);
    newObject->classType = type;
    newObject->previousScope = NULL;
    newObject->nextObject = storage->topScope->nextObject;
    storage->topScope->nextObject = newObject;

}

void enterKeyTab(int symbol, char *name, struct symbolLex **keyTab, int *index) {

    //инициализация элемента keyTab
    keyTab[*index]->symbol = symbol;
    strcpy(keyTab[*index]->identifier, name);
    (*index)++;

}

void initLexical(struct parameters *storage) {

    //инициализация структуры для сканнера
    int i;
    for(i = 0; i < 34; i++)
        storage->keyTab[i] = (struct symbolLex*)malloc(sizeof(struct symbolLex));
    i = 0;
    enterKeyTab(nullLexical, "BY", storage->keyTab, &i);
    enterKeyTab(doLexical, "DO", storage->keyTab, &i);
    enterKeyTab(ifLexical, "IF", storage->keyTab, &i);
    enterKeyTab(nullLexical, "IN", storage->keyTab, &i);
    enterKeyTab(nullLexical, "IS", storage->keyTab, &i);
    enterKeyTab(ofLexical, "OF", storage->keyTab, &i);
    enterKeyTab(orLexical, "OR", storage->keyTab, &i);
    enterKeyTab(nullLexical, "TO", storage->keyTab, &i);
    enterKeyTab(endLexical, "END", storage->keyTab, &i);
    enterKeyTab(nullLexical, "FOR", storage->keyTab, &i);
    enterKeyTab(modLexical, "MOD", storage->keyTab, &i);
    enterKeyTab(nullLexical, "NIL", storage->keyTab, &i);
    enterKeyTab(varLexical, "VAR", storage->keyTab, &i);
    enterKeyTab(nullLexical, "CASE", storage->keyTab, &i);
    enterKeyTab(elseLexical, "ELSE", storage->keyTab, &i);
    enterKeyTab(nullLexical, "EXIT", storage->keyTab, &i);
    enterKeyTab(thenLexical, "THEN", storage->keyTab, &i);
    enterKeyTab(typeLexical, "TYPE", storage->keyTab, &i);
    enterKeyTab(nullLexical, "WITH", storage->keyTab, &i);
    enterKeyTab(arrayLexical, "ARRAY", storage->keyTab, &i);
    enterKeyTab(beginLexical, "BEGIN", storage->keyTab, &i);
    enterKeyTab(constLexical, "CONST", storage->keyTab, &i);
    enterKeyTab(elsifLexical, "ELSIF", storage->keyTab, &i);
    enterKeyTab(nullLexical, "IMPORT", storage->keyTab, &i);
    enterKeyTab(nullLexical, "UNTIL", storage->keyTab, &i);
    enterKeyTab(whileLexical, "WHILE", storage->keyTab, &i);
    enterKeyTab(recordLexical, "RECORD", storage->keyTab, &i);
    enterKeyTab(nullLexical, "REPEAT", storage->keyTab, &i);
    enterKeyTab(nullLexical, "RETURN", storage->keyTab, &i);
    enterKeyTab(nullLexical, "POINTER", storage->keyTab, &i);
    enterKeyTab(procedureLexical, "PROCEDURE", storage->keyTab, &i);
    enterKeyTab(divLexical, "DIV", storage->keyTab, &i);
    enterKeyTab(nullLexical, "LOOP", storage->keyTab, &i);
    enterKeyTab(moduleLexical, "MODULE", storage->keyTab, &i);

}

void initTypes(struct parameters* storage) {

    //инициализация базовых типов. int и bool. их размер - 4
    storage->boolType = (struct type*)malloc(sizeof(struct type));
    storage->boolType->classType = BooleanGen;
    storage->boolType->size = 4;
    storage->intType = (struct type*)malloc(sizeof(struct type));
    storage->intType->classType = IntegerGen;
    storage->intType->size = 4;

}

void initScopes(struct parameters* storage) {

    //инициализация областей видимости - scopes.
    //topScope - указатель на "верхний" scope
    //guard - "закрывающий" элемент. нужен для удобного поиска элементов
    //universe - внешний scope
    //в dsc хранится предыдущий scope
    //в next хранятся объекты текущего scope
    storage->topScope = (struct object*)malloc(sizeof(struct object));
    storage->guard = (struct object*)malloc(sizeof(struct object));
    storage->universe = (struct object*)malloc(sizeof(struct object));
    storage->guard->class = VarGen;
    storage->guard->classType = storage->intType;
    storage->guard->value = 0;
    storage->topScope = NULL;
    openScope(storage); //открытие universe
    enterUniverse(TypGen, 1, "BOOLEAN", storage->boolType, storage);
    enterUniverse(TypGen, 2, "INTEGER", storage->intType, storage);
    enterUniverse(ConstGen, 1, "TRUE", storage->boolType, storage);
    enterUniverse(ConstGen, 0, "FALSE", storage->boolType, storage); //инициализация типов
    enterUniverse(SProcGen, 1, "Read", NULL, storage);
    enterUniverse(SProcGen, 2, "Write", NULL, storage);
    enterUniverse(SProcGen, 3, "WriteHex", NULL, storage);
    enterUniverse(SProcGen, 4, "WriteLn", NULL, storage); //добавление глобальных процедур в universe
    storage->universe = storage->topScope;

}

int init(struct parameters* storage, char* sourceCode) {

    //инициализация
    storage->sourceCode = sourceCode;
    storage->lastPosition = 0;
    strcpy(storage->lastLexeme, "\0");
    storage->lastLexemeCode = -1;
    storage->lastLexemeValue = -1;
    storage->error = 0;
    storage->errorPosition = -1;
    storage->reportFile = NULL;
    if((storage->reportFile = fopen("report.txt", "wb")) == NULL) {
        printf("Opening report.txt file error!\n");
        return -1;
    }
    FILE* outputFile = NULL;
    if((outputFile = fopen("output.txt", "wb")) == NULL) {
        printf("Opening output.txt file error!\n");
        return -1;
    }
    else
        fclose(outputFile); //очистка файла output.txt
    initLexical(storage); //инициализация keyTab
    storage->currentLevel = 0;
    storage->PC = 0;
    memset(storage->registers, 0, sizeof(int) * 16); //обнуление регистров. 0 - не занят
    initTypes(storage); //инициализация базовых типов
    initScopes(storage); //инициализация topScope, guard, universe
    for(int i = 0; i < maxCodeSize; i++)
        storage->code[i] = 0;
    storage->entryAddress = 0;
    storage->posCounter = 0;
    storage->linesCounter = 0;
    return 0;

}

struct object* createNewObject(int class, struct parameters* storage) {

    //создание нового объекта в текущем scope
    struct object* newObject = (struct object*)malloc(sizeof(struct object));
    struct object* buffer;
    buffer = storage->topScope;
    strcpy(storage->guard->name, storage->lastLexeme);
    while(strcmp(buffer->nextObject->name, storage->lastLexeme))
        buffer = buffer->nextObject;
    if(buffer->nextObject == storage->guard) { //если дошли до guard - такого объекта в этом scope нет - создание объекта
        strcpy(newObject->name, storage->lastLexeme);
        newObject->class = class;
        newObject->nextObject = storage->guard;
        buffer->nextObject = newObject;
    }
    else { //иначе - ошибка
        newObject = buffer->nextObject;
        mark("Declared already", storage);
    }
    strcpy(storage->guard->name, "\0"); //"обнуление" guard
    return newObject;

}

struct object* findObject(struct parameters* storage) {

    //поиск объекта во всех scope
    struct object* bufferHead;
    struct object* buffer;
    struct object* object = (struct object*)malloc(sizeof(struct object));
    bufferHead = storage->topScope;
    strcpy(storage->guard->name, storage->lastLexeme);
    do {
        buffer = bufferHead->nextObject;
        while(strcmp(buffer->name, storage->lastLexeme))
            buffer = buffer->nextObject;
        if(buffer != storage->guard)
            object = buffer; //если объект в scope найден
        else if(bufferHead == storage->universe) { //если не найден и уже на внешнем scope
            object = buffer;
            mark("Not declared", storage);
            buffer = NULL; //для выхода из цикла
        }
        else
            bufferHead = bufferHead->previousScope; //иначе на уровень ниже
    } while(buffer == storage->guard);
    strcpy(storage->guard->name, "\0"); //"обнуление" guard
    return object;

}

struct item* selector(struct item* item, struct parameters* storage) { //передается уже инициализированный элемент

    //чтение элемента массива / поля записи
    struct item* newItem = item; //возвращаемый item
    //если нет обращения к элементам, то это просто переменная
    while((storage->lastLexemeCode == lbrakLexical) || (storage->lastLexemeCode == periodLexical)) { // [ либо .
        if(storage->lastLexemeCode == lbrakLexical) { //обращение к элементу массива
            get(storage);
            struct item* indexExpression; //для выражения индекса массива
            indexExpression = expression(storage); //x[y]. получение y
            if(newItem->classType->classType == ArrayGen)
                arrayElem(newItem, indexExpression, storage); //если переменная - массив, получение значение по индексу
            else
                mark("Not an array", storage); //иначе - ошибка
            if(storage->lastLexemeCode == rbrakLexical)
                get(storage);
            else
                mark("]?", storage);
        } //если обращение к элементу массива
        else {
            get(storage);
            if(storage->lastLexemeCode == identLexical) { //x.y. y - всегда индентификатор!
                if(newItem->classType->classType == RecordGen) { //если тип x - запись
                    struct object* fieldObject; //для поиска поля записи
                    fieldObject = findField(newItem->classType->fields, storage); //поиск, если ли такое поле у x
                    get(storage);
                    if(fieldObject != storage->guard) {
                        getField(newItem, fieldObject); //если есть - запись
                    }
                    else
                        mark("Undef record field", storage);
                }
                else
                    mark("Not a record", storage);
            }
            else
                mark("Identifier?", storage);
        } //если обращение к элементу записи
    }
    return newItem;

}

struct item* factor(struct parameters* storage) { //множители

    //анализ идентификатора либо числа
    struct item* currentItem;
    struct object* currentObject;
    if(storage->lastLexemeCode < lparenLexical) {
        mark("Must be identifier or number", storage);
        do {
            get(storage);
        } while(storage->lastLexemeCode < lparenLexical);
    } //проверка допустимость символов
    if(storage->lastLexemeCode == identLexical) {
        currentObject = findObject(storage); //поиск объекта в существующих
        get(storage);
        currentItem = makeItem(currentObject, storage); //создание соответствующего item
        currentItem = selector(currentItem, storage); //либо тот же идент, либо элемент массива, либо поле записи
    } //идентификатор
    else if(storage->lastLexemeCode == numberLexical) {
        currentItem = makeConstItem(storage->intType, storage->lastLexemeValue); //создание item с постоянным значением
        get(storage);
    } //число
    else if(storage->lastLexemeCode == lparenLexical) {
        get(storage);
        currentItem = expression(storage); //( *expression* )
        if(storage->lastLexemeCode == rparenLexical)
            get(storage);
        else
            mark("Lost )", storage);
    } //выражение в скобках
    else if(storage->lastLexemeCode == notLexical) {
        get(storage);
        currentItem = factor(storage);
        singleGenerate(notLexical, currentItem, storage); //выражение со знаком -
    } //отрицание
    else {
        mark("Factor?", storage);
        currentItem = makeItem(storage->guard, storage); //возвращение guard, если нет допустимой переменной
    }
    return currentItem;

}

struct item* term(struct parameters* storage) {

    //анализ выражений: x * y, x DIV y, x MOD y, x & y
    struct item* leftExpression;
    int sign;
    leftExpression = factor(storage); //получение левого выражения
    while((storage->lastLexemeCode >= timesLexical) && (storage->lastLexemeCode <= andLexical)) {
        sign = storage->lastLexemeCode;
        get(storage);
        if(storage->lastLexemeCode == andLexical)
            singleGenerate(sign, leftExpression, storage); //загрузка в регистр для лог. операции
        struct item* rightExpression;
        rightExpression = factor(storage); //получение правого выражения
        termGenerate(sign, leftExpression, rightExpression, storage); //получение выражения с учетом знака
    } //для умножения, div, mod, &
    return leftExpression;

}

struct item* simpleExpression(struct parameters* storage) {

    //анализ суммы, разности, |. учитывается знак перед первым аргументом (- или +)
    struct item* leftExpression;
    int sign;
    if(storage->lastLexemeCode == plusLexical) {
        get(storage);
        leftExpression = term(storage); //получение левого выражения
    } //+item
    else if(storage->lastLexemeCode == minusLexical){
        get(storage);
        leftExpression = term(storage);
        singleGenerate(minusLexical, leftExpression, storage); //выражение со знаком -
    } //-item
    else {
        leftExpression = term(storage);
    } //нет знака
    while((storage->lastLexemeCode >= plusLexical) && (storage->lastLexemeCode <= orLexical)) {
        sign = storage->lastLexemeCode; //получение знака
        get(storage);
        if(sign == orLexical)
            singleGenerate(sign, leftExpression, storage); //загрузка в регистры для лог. операции
        struct item* rightExpression = term(storage); //получение правого выражения
        termGenerate(sign, leftExpression, rightExpression, storage); //получение выражения с учетом знака
    } //анализ суммы/разности/OR
    return leftExpression;

}

struct item* expression(struct parameters* storage) {

    //анализ либо переменной, либо лог. выражения x ( = | # | < | <= | > | >= ) y
    int sign;
    struct item* leftExpression = simpleExpression(storage); //получение левого выражения
    if((storage->lastLexemeCode >= eqlLexical) && (storage->lastLexemeCode <= gtrLexical)) {
        sign = storage->lastLexemeCode;
        get(storage);
        struct item* rightExpression = simpleExpression(storage); //получение правого выражение
        relation(sign, leftExpression, rightExpression, storage); //получение выражения с учетом знака
    } //если след. знак ( = | # | < | <= | > | >= )
    return leftExpression;

}

struct type* getType(struct parameters* storage) {

    //чтение типа. (array, record, intType, boolType, созданный тип)
    struct type* type = (struct type*)malloc(sizeof(struct type)); //полученный тип
    if((storage->lastLexemeCode != identLexical) && (storage->lastLexemeCode < arrayLexical)) {
        mark("No type?", storage);
        do {
            get(storage);
        } while((storage->lastLexemeCode != identLexical) && (storage->lastLexemeCode < arrayLexical));
    } //проверка допустимых символов
    if(storage->lastLexemeCode == identLexical) { //если это объявленный тип
        struct object* objectBuffer;
        objectBuffer = findObject(storage); //поиск в существующих объектах
        get(storage);
        if(objectBuffer->class == TypGen) //если класс найденного объекта - тип, то присваивание
            type = objectBuffer->classType;
        else
            mark("Type identifier?", storage);
    }
    else if(storage->lastLexemeCode == arrayLexical) { //ARRAY *expression* OF *type*
        get(storage);
        struct item* expressionItem;
        expressionItem = expression(storage); //получение индекса
        if((expressionItem->mode != ConstGen) || (expressionItem->a < 0)) //размер массива - ПОСТОЯННАЯ, значение - НЕ ОТРИЦАТЕЛЬНОЕ
            mark("Bad index", storage);
        if(storage->lastLexemeCode == ofLexical)
            get(storage);
        else
            mark("Lost OF", storage);
        type->classType = ArrayGen;
        type->baseType = getType(storage); //баз. тип
        type->length = expressionItem->a; // "длина" массива
        type->size = type->length * type->baseType->size; //размер массива = длина * размер типа
    }
    else if(storage->lastLexemeCode == recordLexical) { //RECORD *ident1*, *ident2* : *type1* {; *identN* : *typeN*} END
        get(storage);
        type->classType = RecordGen;
        type->size = 0;
        openScope(storage); //открытие scope для записи
        while((storage->lastLexemeCode == semicolonLexical) || (storage->lastLexemeCode == identLexical)) { //чтение полей
            if(storage->lastLexemeCode == identLexical) {
                struct object* objectBuffer;
                struct object* listHeader;
                listHeader = identifiersList(FldGen, storage); //запись объектов поля в scope
                struct type* typeBuffer;
                typeBuffer = getType(storage); //получение типа поля
                objectBuffer = listHeader;
                while(objectBuffer != storage->guard) { //запись типа для всех идентификаторов
                    objectBuffer->classType = typeBuffer;
                    objectBuffer->value = type->size; //значение - смещение относительно базового адреса записи
                    type->size += typeBuffer->size; //увеличение смещения
                    objectBuffer = objectBuffer->nextObject;
                }
            }
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else if(storage->lastLexemeCode == identLexical)
                mark("Lost ;", storage);
        }
        type->fields = storage->topScope->nextObject; //->fields - указатель на первое поле. структура как в scope
        closeScope(storage); //закрытие scope
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("Lost END", storage);
    }
    else
        mark("Identifier type?", storage);
    if(type != NULL)
        return type;
    else
        return storage->intType; //если тип не идентифицирован, считается, что это int

}

struct object* identifiersList(int class, struct parameters *storage) {

    //чтение всех идентификаторов вида ident1, ... , ident2 : . возврат указателя на первый элемент
    struct object* headerObject = (struct object*)malloc(sizeof(struct object));
    if(storage->lastLexemeCode == identLexical) {
        headerObject = createNewObject(class, storage); //в first первый идентификатор
        get(storage);
        while(storage->lastLexemeCode == commaLexical) { //пока , . ident1, ident2, ...
            get(storage);
            if(storage->lastLexemeCode == identLexical) {
                createNewObject(class, storage);
                get(storage);
            }
            else
                mark("Identifier?", storage);
        }
        if(storage->lastLexemeCode == colonLexical)
            get(storage);
        else
            mark("Lost :", storage);
    }
    return headerObject;

}

void openScope(struct parameters* storage) {

    //открытие нового scope
    struct object* newScope = (struct object*)malloc(sizeof(struct object));
    newScope->class = HeadGen; //первый элемент scope - head
    newScope->previousScope = storage->topScope; //dsc - указатель на предыдущий scope
    newScope->nextObject = storage->guard; //след. элемента нет
    storage->topScope = newScope; //новая вершина scopes

}

int declarations(struct parameters* storage) { //возвращается размер переменных

    //чтение объявлений CONST, TYPE, VAR
    int varSize = 0;
    while((storage->lastLexemeCode >= constLexical) && (storage->lastLexemeCode <= varLexical)) { //пока const, type, var
        if(storage->lastLexemeCode == constLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //CONST *ident1* = *expr*; *ident2* = *expr*; ...
                struct object* currentObject; //структура для рассматриваемого объекта
                struct item* currentExpression; //структура для анализа выражения
                currentObject = createNewObject(ConstGen, storage); //создание объекта для константы
                get(storage);
                if(storage->lastLexemeCode == eqlLexical)
                    get(storage);
                else
                    mark("Lost :=", storage); //так как это const, должна быть инициализация
                currentExpression = expression(storage); // CONST *ident1* = *expr*;
                if(currentExpression->mode == ConstGen) {
                    currentObject->value = currentExpression->a; //поле значения
                    currentObject->classType = currentExpression->classType; //поле типа
                }
                else
                    mark("Expression is not a constant", storage);
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark("Lost ;", storage); //после каждой константы ;
            }
        }
        if(storage->lastLexemeCode == typeLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //TYPE *ident1* = *type*; *ident2* = *type*; ...
                struct object* currentObject = createNewObject(TypGen, storage); //создание объекта типа
                get(storage);
                if(storage->lastLexemeCode == eqlLexical)
                    get(storage);
                else
                    mark("Lost :=", storage); //должно быть объявление типа
                currentObject->classType = getType(storage); //получение объявления типа
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark("Lost ;", storage); //после каждого типа ;
            }
        }
        if(storage->lastLexemeCode == varLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //*identList1* : *type; *identList2* : *type*; ...
                struct object* currentObject; //структура для рассматриваемого объекта
                struct object* identListHead; //для множества идентификаторов. указатель на первый элемент
                struct type* currentType; //тип переменной
                identListHead = identifiersList(VarGen, storage); //получение списка ВСЕХ идентификаторов через запятую, запись
                                                    //в текущий scope, получение указателя на первый записанный объект
                currentType = getType(storage); //получение типа
                currentObject = identListHead;
                while(currentObject != storage->guard) { //для всех полученных объектов
                    varSize += currentType->size; //размер типа + к varSize
                    currentObject->classType = currentType; //тип
                    currentObject->level = storage->currentLevel; //текущий уровень. для отличия переменных разных scope
                    currentObject->value = -varSize; //смещение относительно базового адреса
                    currentObject = currentObject->nextObject;
                }
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark("Lost ;", storage); //после каждой переменной ;
            }
        }
        if((storage->lastLexemeCode >= constLexical) && (storage->lastLexemeCode <= varLexical))
            mark("Wrong declarations order", storage); //const->type->var !
    }
    return varSize;

}

int parametersBlockAnalyzer(struct parameters *storage) {

    //чтение блока формальных параметров функции
    struct object* listHead; //указатель на первый объект
    struct object* objectBuffer;
    struct type* currentType; //тип параметров
    int typeSize; //размер типа
    int parametersSize = 0; //размер блока
    listHead = identifiersList(VarGen, storage); //VAR - VarGen, - параметр-значение
    if(storage->lastLexemeCode == identLexical) { //ident - тип переменных
        objectBuffer = findObject(storage); //поиск типа в существующих
        get(storage);
        if(objectBuffer->class == TypGen)
            currentType = objectBuffer->classType; //если тип существует - присваивание
        else {
            mark("Identifier is not a type", storage);
            currentType = storage->intType; //иначе - ошибка. присваевание int
        }
    }
    else {
        mark("Identifier type?", storage);
        currentType = storage->intType; //если тип не указан - присваивание int
    }
    typeSize = currentType->size;
    if(currentType->classType >= ArrayGen)
        mark("Incorrect type of parameter", storage); //параметром может быть только int и bool
    objectBuffer = listHead;
    while(objectBuffer != storage->guard) { //инициализация типа для всех новых объектов. подсчет их размера
        objectBuffer->classType = currentType;
        parametersSize += typeSize;
        objectBuffer = objectBuffer->nextObject;
    }
    return parametersSize; //возврат размера блока

}

void procedureAnalyzer(struct parameters *storage) {

    //анализ процедур. PROCEDURE *ident*(*FormalParameters*) *ProcBody*
    struct object* currentProcedure; //объект процедуры
    char procedureName[identLength]; //имя процедуры
    int currentBlockSize, parametersSize;
    int markSize = 8; //размер "следа" процедуры
    get(storage);
    if(storage->lastLexemeCode == identLexical) { //PROCEDURE *ident*
        strcpy(procedureName, storage->lastLexeme); //получение идент. процедуры
        currentProcedure = createNewObject(ProcGen, storage); //создание нового объекта для процедуры в текущем scope
        get(storage);
        parametersSize = markSize; //размер параметров - след процедуры
        changeLevel(1, storage); //увеличение уровня вложенности
        openScope(storage); //открытие scope для новой процедуры
        currentProcedure->value = -1;
        if(storage->lastLexemeCode == lparenLexical) { //параметры функции (?? : ??; ?? : ??; ...)
            get(storage);
            if(storage->lastLexemeCode == rparenLexical) //если ()
                get(storage);
            else {
                parametersSize += parametersBlockAnalyzer(storage); //чтение ident1, ident2, .. : type. увеличение размера блока параметров
                while(storage->lastLexemeCode == semicolonLexical) { //между 2 и > должно быть ;
                    get(storage);
                    parametersSize += parametersBlockAnalyzer(storage);
                }
                if(storage->lastLexemeCode == rparenLexical)
                    get(storage);
                else
                    mark("Lost )", storage);
            }
        }
        struct object* objectBuffer;
        objectBuffer = storage->topScope->nextObject; //переход к первому параметру
        currentBlockSize = parametersSize;
        while(objectBuffer != storage->guard) { //полю val каждого объекта присваивается значение смещения относительно
                                          //кадра активации. смещение положительное
            objectBuffer->level = storage->currentLevel;
            if(objectBuffer->class == ParGen)
                currentBlockSize -= wordSize;
            else
                currentBlockSize -= objectBuffer->classType->size; //инкремент. смещение каждую итерацию
            objectBuffer->value = currentBlockSize; //присваивание смещения
            objectBuffer = objectBuffer->nextObject;
        }
        currentProcedure->previousScope = storage->topScope->nextObject; //дескриптор процесса - его параметры
        if(storage->lastLexemeCode == semicolonLexical)
            get(storage);
        else
            mark(";?", storage); //после PROCEDURE ident() должно быть ;
        currentBlockSize = declarations(storage); //лок. параметры процедуры
        while(storage->lastLexemeCode == procedureLexical) {
            procedureAnalyzer(storage);
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                mark(";?", storage);
        } //внутр. функции функции
        currentProcedure->value = storage->PC; //value - точка входа в процедуру
        prologue(currentBlockSize, storage); //запись пролога процедуры
        if(storage->lastLexemeCode == beginLexical) { //тело процедуры BEGIN procbody END *procident*
            get(storage);
            statements(storage); //последовательность операторов
        }
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("Lost END", storage);
        if(storage->lastLexemeCode == identLexical) {
            if(strcmp(procedureName, storage->lastLexeme))
                mark("No match procedure identifier", storage); //идент. в конце должен совпадать с идент. процедуры
            get(storage);
        }
        epilogue(parametersSize - markSize, storage); //запись эпилога процедуры
        closeScope(storage); //закрытие scope
        changeLevel(-1, storage); //-1 уровень вложенности
    }

}

struct object* parametersMatch(struct object *formalParameter, struct parameters *storage) {

    //проверка параметров процедуры на корректность
    struct item* parameterExpression;
    struct object* nextFormalParameter;
    nextFormalParameter = formalParameter;
    parameterExpression = expression(storage); //получение выражения параметра
    if(parameterFormat(nextFormalParameter)) { //если параметр корректен
        parameterGen(parameterExpression, nextFormalParameter->classType, storage); //запись параметра в стек
        nextFormalParameter = nextFormalParameter->nextObject; //переход к след. параметру
    }
    else
        mark("Wrong parameters", storage);
    return nextFormalParameter;

}

int parameterFormat(struct object *object) {

    //проверка, является ли объект допустимым параметром (проверка только формата)
    //параметр - Par либо Var. значение val - положительное, так как положительное смещение
    return ((object->class == ParGen) || ((object->class == VarGen) && (object->value > 0)));

}

struct item* getSingleParameter(struct parameters *storage) {

    //получение выражения для глоб. процедур. ( ВЫРАЖЕНИЕ )
    struct item* currentParameter;
    if(storage->lastLexemeCode == lparenLexical)
        get(storage);
    else
        mark("(?", storage);
    currentParameter = expression(storage);
    if(storage->lastLexemeCode == rparenLexical)
        get(storage);
    else
        mark(")?", storage);
    return currentParameter;

}

void statements(struct parameters *storage) {

    //последовательность операторов. *оператор* {; *оператор*}
    //оператор: присваивание / вызов процедуры / If / While
    do {
        struct object* currentObject;
        if(storage->lastLexemeCode < identLexical) {
            mark("Statement?", storage);
            do {
                get(storage);
            } while(storage->lastLexemeCode < identLexical);
        } //поиск по коду, пока не будет корректных символов
        if(storage->lastLexemeCode == identLexical) { //если слово - идентификатор
            currentObject = findObject(storage); //поиск идент. среди объявленных
            get(storage);
            struct item* currentItem;
            currentItem = makeItem(currentObject, storage); //создание item для вычисления выражения
            currentItem = selector(currentItem, storage); //получение значения по индексу/полю, если они есть
            if(storage->lastLexemeCode == becomesLexical) { //x := y присваивание
                get(storage);
                struct item* bufferItem;
                bufferItem = expression(storage);
                store(currentItem, bufferItem, storage); //запись y в x
            }
            else if(storage->lastLexemeCode == eqlLexical) {
                mark("Lost : ?", storage);
                get(storage);
                expression(storage);
            } //если =, предполагается, что пропущено :
            else if(currentItem->mode == ProcGen) { //если объект - процедура, то это ее вызов
                struct object* parametersObject = currentObject->previousScope; //получение формальных параметров процедуры. 
                                                                 // parametersObject - указатель на первый
                if(storage->lastLexemeCode == lparenLexical) { //procedure(params). чтение фактических параметров
                    get(storage);
                    if(storage->lastLexemeCode == rparenLexical)
                        get(storage); //если параметров нет
                    else { //чтение передаваемых параметров
                        do {
                            parametersObject = parametersMatch(parametersObject, storage); //проверка и запись передаваемых параметров
                            if(storage->lastLexemeCode == commaLexical)
                                get(storage); //неск. параметров - запятые
                            else if(storage->lastLexemeCode < semicolonLexical) {
                                if(storage->lastLexemeCode != rparenLexical)
                                    mark(") or , ?", storage); //если встречен недопустимый символ и это не ) - ошибка
                                else
                                    get(storage); //если ) - чтение след. символа - выход из цикла
                            }
                        } while((storage->lastLexemeCode != rparenLexical) && (storage->lastLexemeCode < semicolonLexical));
                    }
                }
                if(currentObject->value < 0)
                    mark("Forward procedure call", storage); //если вызов процедуры выше своего scope
                else if(!parameterFormat(parametersObject)) //если все параметры корректны - запись вызова процедуры
                    procedureCall(currentItem, storage);
                else
                    mark("Wrong parameters", storage); //иначе - ошибка
            }
            else if(currentItem->mode == SProcGen) { //если вызов одной из объявленных в universe процедур: Read, Write, WriteHex, WriteLn
                struct item* bufferItem;
                if(currentObject->value <= 3) //для каждой процедуры свое значение: 1, 2, 3, 4. Для 1, 2, 3 есть аргументы,
                    bufferItem = getSingleParameter(storage); //для WriteLn - нет. здесь получение аргумента
                globalCall(currentItem, bufferItem, storage); //запись вызова
            }
            else if(currentObject->class == TypGen)
                mark("Illegal type assignment", storage); //если оператор начинается с типа
            else
                mark("Illegal statement", storage);
        }
        else if(storage->lastLexemeCode == ifLexical) { //если цикл if
            get(storage);
            struct item* currentItem;
            currentItem = expression(storage); //анализ выражения-условия ( exp )
            falseJump(currentItem, storage); //переход через блок, если ложно
            if(storage->lastLexemeCode == thenLexical)
                get(storage);
            else
                mark("Lost THEN", storage); //IF *exp* THEN
            statements(storage); //последовательность операторов
            int jumpLevel = 0; //уровень ветвления переходов
            while(storage->lastLexemeCode == elsifLexical) {
                get(storage);
                jumpLevel = elseJump(jumpLevel, storage); //запись адреса для списка переходов if
                fixLink(currentItem->a, storage); //запись адреса для предыдущих операторов
                currentItem = expression(storage); //получение очередного условия перехода
                falseJump(currentItem, storage); //переход, если ложно
                if(storage->lastLexemeCode == thenLexical)
                    get(storage);
                else
                    mark("Lost THEN", storage); //ELSIF *exp* THEN
                statements(storage); //последовательность операторов
            }
            if(storage->lastLexemeCode == elseLexical) { //последнее условие
                get(storage);
                jumpLevel = elseJump(jumpLevel, storage); //запись адреса для списка переходов if
                fixLink(currentItem->a, storage); //запись адреса для предыдущих операторов
                statements(storage); //последовательность операторов
            }
            else
                fixLink(currentItem->a, storage); //запись адреса для предыдущих операторов
            fixLink(jumpLevel, storage); //выравнивание адресов всего списка переходов
            if(storage->lastLexemeCode == endLexical)
                get(storage);
            else
                mark("Lost END", storage); //IF *...* END !
        }
        else if(storage->lastLexemeCode == whileLexical) { //WHILE *cond* DO *statements* END
            get(storage);
            int jumpAddress = storage->PC; //адрес возврата цикла
            struct item* currentItem;
            currentItem = expression(storage); //получение условие
            falseJump(currentItem, storage); //переход, если условие не выполняется
            if(storage->lastLexemeCode == doLexical)
                get(storage);
            else
                mark("Lost DO", storage);
            statements(storage); //последовательность операторов
            whileJump(jumpAddress, storage); //безусловный переход к началу цикла
            fixLink(currentItem->a, storage); //выравнивание адреса перехода к концу цикла
            if(storage->lastLexemeCode == endLexical)
                get(storage);
            else
                mark("Lost END", storage);
        }
        if(storage->lastLexemeCode == semicolonLexical)
            get(storage);
        else if((storage->lastLexemeCode <= identLexical) || (storage->lastLexemeCode == ifLexical) || (storage->lastLexemeCode == whileLexical)) {
            mark(";?", storage); //если идент, if или while, но между ними нет ;
        }

    } while((storage->lastLexemeCode <= identLexical) || (storage->lastLexemeCode == ifLexical) || (storage->lastLexemeCode == whileLexical));

}

void closeScope(struct parameters* storage) {

    //закрытие текущей области видимости
    storage->topScope = storage->topScope->previousScope; //возврат к предыдущему scope

}

int module(struct parameters* storage) {

    //чтение модуля
    signal("Compilation begins.", storage);
    if(storage->lastLexemeCode == moduleLexical) { //модуль начинается с MODULE
        char moduleName[identLength] = "\0"; //название модуля
        int varSize = 0; //размер памяти для глобальных переменных
        get(storage);
        openScope(storage); //открытие нового scope
        if(storage->lastLexemeCode == identLexical) { //MODULE *ident*
            strcpy(moduleName, storage->lastLexeme);
            //"Compiles module moduleName."
            fprintf(storage->reportFile, "Compiles module %s.\r\n", moduleName);
            printf("Compiles module %s.\n", moduleName);
            get(storage);
        }
        else
            mark("No module identifier", storage);
        if(storage->lastLexemeCode == semicolonLexical) //MODULE *ident*;
            get(storage);
        else
            mark("Lost ;", storage);
        varSize = declarations(storage); //чтение объявлений CONST, TYPE, VAR. varSize - размер переменных
        while(storage->lastLexemeCode == procedureLexical) { //чтение процедуры
            procedureAnalyzer(storage);
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                mark("Lost ;", storage); //PROCEDURE procbody; !
        }
        codeHeader(varSize, storage); //пролог. запись точки входа и указателя на вершину стека
        if(storage->lastLexemeCode == beginLexical) { //начало модуля
            get(storage);
            statements(storage); //анализ последовательности операторов
        }
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("Lost END", storage); //конец модуля
        if(storage->lastLexemeCode == identLexical) {
            if(strcmp(moduleName, storage->lastLexeme))
                mark("Wrong module identifier", storage);
            get(storage);
        }
        else
            mark("No module identifier", storage);
        if(storage->lastLexemeCode != periodLexical)
            mark("Lost .", storage); //MODULE *ident*.
        closeScope(storage); //закрытие universe
        if(!storage->error) { //если не было ошибок - генерация кода
            codeEnding(storage); //эпилог. возврат вершины стека
            signal("Code generated.", storage);
            return 0;
        }
    }
    else
        mark("No MODULE", storage);
    return 1; //возвращается 1, если ошибка

}

void compile(char *sourceCode) {

    struct parameters* storage =
            (struct parameters*)malloc(sizeof(struct parameters));
    if(init(storage, sourceCode) != 0)
        return; //важно, потому что могут быть ошибки обращения к памяти
    get(storage); //получение первого символа
    if (module(storage) == 0) //анализ кода, компиляция
        decode(storage); //если компиляция прошла успешно - запись кода в output.txt
    signal("Compilation finished.", storage);

}