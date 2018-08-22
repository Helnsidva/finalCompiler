#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "scanner.h"
#include "codeGenerator.h"

void openScope(struct parameters*);
void closeScope(struct parameters*);
void statements(struct parameters*);
struct Item* expression(struct parameters*);
struct Object* identifiersList(int, struct parameters*);
int parameterFormat(struct Object*);

void signal(char msg[], struct parameters* storage) {

    //печать сообщения в report.txt и в консоль
    fprintf(storage->reportFile, "%s\r\n", msg);
    printf("%s\n", msg);

}

void enterUniverse(int class, int value, char *name, struct Type *type, struct parameters *storage) {

    //создание объектов в universe
    struct Object* newObject = (struct Object*)malloc(sizeof(struct Object));
    newObject->class = class;
    newObject->value = value;
    strcpy(newObject->name, name);
    newObject->type = type;
    newObject->previousScope = NULL;
    newObject->nextObject = storage->topScope->nextObject;
    storage->topScope->nextObject = newObject;

}

void enterKeyTab(int symbol, char *name, struct keyLex **keyTab, int *index) {

    //инициализация элемента keyTab
    keyTab[*index]->symbol = symbol;
    strcpy(keyTab[*index]->identifier, name);
    (*index)++;

}

void initLexical(struct parameters *storage) {

    //инициализация структуры для сканнера
    int i = 0;
    for(i; i < 34; i++) {
        storage->keyTab[i] = (struct keyLex*)malloc(sizeof(struct keyLex));
    }
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

    //инициализация базовых типов. int и bool. их размер = 4
    storage->boolType = (struct Type*)malloc(sizeof(struct Type));
    storage->boolType->type = BooleanGen;
    storage->boolType->size = 4;
    storage->intType = (struct Type*)malloc(sizeof(struct Type));
    storage->intType->type = IntegerGen;
    storage->intType->size = 4;

}

void initScopes(struct parameters* storage) {

    //инициализация областей видимости - scopes.
    //topScope - указатель на "верхний" scope
    //guard - "закрывающий" элемент. нужен для удобного поиска элементов
    //universe - внешний scope
    //в dsc хранится предыдущий scope
    //в next хранятся объекты текущего scope
    storage->topScope = (struct Object*)malloc(sizeof(struct Object));
    storage->guard = (struct Object*)malloc(sizeof(struct Object));
    storage->universe = (struct Object*)malloc(sizeof(struct Object));
    storage->guard->class = VarGen;
    storage->guard->type = storage->intType;
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

    //инициализация прочего
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
    initLexical(storage);
    storage->linesCounter = 0; //todo считать строки
    storage->currentLevel = 0;
    storage->PC = 0;
    memset(storage->registers, 0, sizeof(int) * 16);
    initTypes(storage);
    initScopes(storage);
    for(int i = 0; i < maxCodeSize; i++)
        storage->code[i] = 0;
    storage->entryAddress = 0;
    return 0;

}

struct Object* createNewObject(int class, struct parameters* storage) {

    //создание нового объекта в текущем scope
    struct Object* newObject = (struct Object*)malloc(sizeof(struct Object));
    struct Object* buffer;
    buffer = storage->topScope;
    strcpy(storage->guard->name, storage->lastLexeme);
    while(strcmp(buffer->nextObject->name, storage->lastLexeme))
        buffer = buffer->nextObject;
    if(buffer->nextObject == storage->guard) { //если дошли до guard - такого объекта в этом scope нет - создаем объект
        strcpy(newObject->name, storage->lastLexeme);
        newObject->class = class;
        newObject->nextObject = storage->guard;
        buffer->nextObject = newObject;
    }
    else { //иначе - ошибка
        newObject = buffer->nextObject;
        mark("Declared already", storage);
    }
    strcpy(storage->guard->name, "\0"); //"обнуляем" guard
    return newObject;

}

struct Object* findObject(struct parameters* storage) {

    //поиск объекта во всех scope
    struct Object* bufferHead;
    struct Object* buffer;
    struct Object* object = (struct Object*)malloc(sizeof(struct Object));
    bufferHead = storage->topScope;
    strcpy(storage->guard->name, storage->lastLexeme);
    do {
        buffer = bufferHead->nextObject;
        while(strcmp(buffer->name, storage->lastLexeme))
            buffer = buffer->nextObject;
        if(buffer != storage->guard)
            object = buffer; //если проверили scope и нашли объект
        else if(bufferHead == storage->universe) { //если не нашли и мы уже на внешнем scope
            object = buffer;
            mark("Not declared", storage);
            buffer = NULL; //для выхода из цикла
        }
        else
            bufferHead = bufferHead->previousScope; //иначе идем на уровень ниже
    } while(buffer == storage->guard);
    strcpy(storage->guard->name, "\0"); //"обнуляем" guard
    return object;

}

struct Item* selector(struct Item* item, struct parameters* storage) { //передается уже инициализированный элемент

    //чтение элемента массива / поля записи
    struct Item* newItem = item; //возвращаемый item
    struct Item* indexExpression; //для выражения индекса массива
    struct Object* fieldObject; //для поиска поля записи
    //если нет обращения к элементам, то это просто переменная
    while((storage->lastLexemeCode == lbrakLexical) || (storage->lastLexemeCode == periodLexical)) { // [ либо .
        if(storage->lastLexemeCode == lbrakLexical) {
            get(storage);
            indexExpression = expression(storage); //x[y]. получаем y
            if(newItem->type->type == ArrayGen)
                Index(newItem, indexExpression, storage); //если переменная - массив, получаем значение по индексу todo
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
                if(newItem->type->type == RecordGen) { //если тип x - запись
                    fieldObject = FindField(newItem->type->fields, storage); //находим, если ли такое поле у x todo
                    get(storage);
                    if(fieldObject != storage->guard) {
                        Field(newItem, fieldObject, storage); //если есть - записываем todo
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

struct Item* factor(struct parameters* storage) { //множители

    //анализ идентификатора либо числа
    struct Item* item;
    struct Object* object;
    if(storage->lastLexemeCode < lparenLexical) {
        mark("Identifier? Number?", storage);
        do {
            get(storage);
        } while(storage->lastLexemeCode < lparenLexical);
    } //проверяем допустимость символов
    if(storage->lastLexemeCode == identLexical) {
        object = findObject(storage); //ищем объект в существующих
        get(storage);
        item = MakeItem(object, storage); //создаем соответствующий item //todo для selector?
        item = selector(item, storage); //либо тот же идент, либо элемент массива, либо поле записи
    } //идентификатор
    else if(storage->lastLexemeCode == numberLexical) {
        item = MakeConstItem(storage->intType, storage->lastLexemeValue, storage); //создает item с постоянным значением
        get(storage);
    } //число
    else if(storage->lastLexemeCode == lparenLexical) {
        get(storage);
        item = expression(storage); //( *expression* )
        if(storage->lastLexemeCode == rparenLexical)
            get(storage);
        else
            mark(")?", storage);
    } //выражение в скобках
    else if(storage->lastLexemeCode == notLexical) {
        get(storage);
        item = factor(storage);
        Op1(notLexical, item, storage); //выражение со знаком -
    } //отрицание
    else {
        mark("Factor?", storage);
        item = MakeItem(storage->guard, storage); //возвращаем guard, если нет допустимой переменной
    }
    return item;

}

struct Item* term(struct parameters* storage) {

    //анализ выражений: x * y, x DIV y, x MOD y, x & y
    struct Item* leftExpression;
    struct Item* rightExpression;
    int sign;
    leftExpression = factor(storage); //получение левого выражения
    while((storage->lastLexemeCode >= timesLexical) && (storage->lastLexemeCode <= andLexical)) {
        sign = storage->lastLexemeCode;
        get(storage);
        if(storage->lastLexemeCode == andLexical)
            Op1(sign, leftExpression, storage); //todo загрузка в регистры для лог. операции?
        rightExpression = factor(storage); //получение правого выражения
        Op2(sign, leftExpression, rightExpression, storage); //получаем выражение с учетом знака
    } //для умножения, div, mod, &
    return leftExpression;

}

struct Item* simpleExpression(struct parameters* storage) {

    //анализ суммы, разности, |. учитывается знак перед первым аргументом (- или +)
    struct Item* leftExpression;
    struct Item* rightExpression;
    int sign;
    if(storage->lastLexemeCode == plusLexical) {
        get(storage);
        leftExpression = term(storage); //получение левого выражения
    } //+item
    else if(storage->lastLexemeCode == minusLexical){
        get(storage);
        leftExpression = term(storage);
        Op1(minusLexical, leftExpression, storage); //выражение со знаком -
    } //-item
    else {
        leftExpression = term(storage);
    } //нет знака
    while((storage->lastLexemeCode >= plusLexical) && (storage->lastLexemeCode <= orLexical)) {
        sign = storage->lastLexemeCode; //получаем знак
        get(storage);
        if(sign == orLexical)
            Op1(sign, leftExpression, storage); //todo загрузка в регистры для лог. операции?
        rightExpression = term(storage); //получение правого выражения
        Op2(sign, leftExpression, rightExpression, storage); //получаем выражение с учетом знака
    } //анализ суммы/разности/OR
    return leftExpression;

}

struct Item* expression(struct parameters* storage) {

    //анализ либо переменной, либо лог. выражения x ( = | # | < | <= | > | >= ) y
    struct Item* leftExpression;
    struct Item* rightExpression;
    int sign;
    leftExpression = simpleExpression(storage); //получаем левое выражение
    if((storage->lastLexemeCode >= eqlLexical) && (storage->lastLexemeCode <= gtrLexical)) {
        sign = storage->lastLexemeCode;
        get(storage);
        rightExpression = simpleExpression(storage); //получаем правое выражение
        Relation(sign, leftExpression, rightExpression, storage); //получаем выражение с учетом знака //todo
    } //если след. знак ( = | # | < | <= | > | >= )
    return leftExpression;

}

struct Type* getType(struct parameters* storage) {

    //чтение типа. (array, record, intType, boolType, созданный тип)
    struct Object* objectBuffer;
    struct Object* listHeader;
    struct Item* expressionItem;
    struct Type* typeBuffer;
    struct Type* type = (struct Type*)malloc(sizeof(struct Type));
    if((storage->lastLexemeCode != identLexical) && (storage->lastLexemeCode < arrayLexical)) {
        mark("type?", storage);
        do {
            get(storage);
        } while((storage->lastLexemeCode != identLexical) && (storage->lastLexemeCode < arrayLexical));
    } //проверяем допустимые символы
    if(storage->lastLexemeCode == identLexical) { //если это объявленный тип
        objectBuffer = findObject(storage); //находим в существующих
        get(storage);
        if(objectBuffer->class == TypGen) //если класс найденного объекта - тип, то присваиваем
            type = objectBuffer->type;
        else
            mark("Type identifier?", storage);
    }
    else if(storage->lastLexemeCode == arrayLexical) { //ARRAY *expression* OF *type*
        get(storage);
        expressionItem = expression(storage);
        if((expressionItem->mode != ConstGen) || (expressionItem->a < 0)) //размер массива - ПОСТОЯННАЯ, значение - НЕ ОТРИЦАТЕЛЬНОЕ
            mark("Bad index", storage);
        if(storage->lastLexemeCode == ofLexical)
            get(storage);
        else
            mark("OF?", storage);
        type->type = ArrayGen;
        type->baseType = getType(storage); //баз. тип
        type->length = expressionItem->a; // "длина" массива
        type->size = type->length * type->baseType->size; //размер массива - длина * размер типа
    }
    else if(storage->lastLexemeCode == recordLexical) { //RECORD *ident1*, *ident2* : *type1* {; *identN* : *typeN*} END
        get(storage);
        type->type = RecordGen;
        type->size = 0;
        openScope(storage);
        while((storage->lastLexemeCode == semicolonLexical) || (storage->lastLexemeCode == identLexical)) { //чтение полей
            if(storage->lastLexemeCode == identLexical) {
                listHeader = identifiersList(FldGen, storage); //записываем объекты в scope
                typeBuffer = getType(storage); //получаем тип
                objectBuffer = listHeader;
                while(objectBuffer != storage->guard) { //записываем тип для всех идентификаторов
                    objectBuffer->type = typeBuffer;
                    objectBuffer->value = type->size; //todo val - адрес?
                    type->size += typeBuffer->size;
                    objectBuffer = objectBuffer->nextObject;
                }
            }
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else if(storage->lastLexemeCode == identLexical)
                mark(";?", storage);
        }
        type->fields = storage->topScope->nextObject; //->fields - указатель на первое поле. структура как в scope
        closeScope(storage); //закрываем scope. использовали его просто как буфер
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("END?", storage);
    }
    else
        mark("Identifier type?", storage);
    if(type != NULL)
        return type;
    else
        return storage->intType;

}

struct Object* identifiersList(int class, struct parameters *storage) {

    //чтение всех идентификаторов вида ident1, ... , ident2 : . возврат указателя на первый элемент
    struct Object* headerObject = (struct Object*)malloc(sizeof(struct Object));
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
            mark(":?", storage);
    }
    return headerObject;

}

void openScope(struct parameters* storage) {

    //открытие нового scope
    struct Object* newScope = (struct Object*)malloc(sizeof(struct Object));
    newScope->class = HeadGen; //первый элемент scope - head
    newScope->previousScope = storage->topScope; //dsc - указатель на предыдущий scope
    newScope->nextObject = storage->guard; //след. элемента нет
    storage->topScope = newScope; //новая вершина scopes

}

int declarations(struct parameters* storage, int argVarSize) { //возвращается новое значение varSize

    //чтение объявлений CONST, TYPE, VAR
    struct Item* currentExpression; //структура для анализа выражений
    struct Object* currentObject; //структура для рассматриваемого объекта
    struct Object* identListHead; //для множества идентификаторов. указатель на первый элемент
    struct Type* currentType; //тип переменной
    int varSize = argVarSize;
    while((storage->lastLexemeCode >= constLexical) && (storage->lastLexemeCode <= varLexical)) { //пока const, type, var
        if(storage->lastLexemeCode == constLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //CONST *ident1* = *expr*; *ident2* = *expr*; ...
                currentObject = createNewObject(ConstGen, storage); //создаем ОБЪЕКТ для каждой переменной.
                get(storage);
                if(storage->lastLexemeCode == eqlLexical)
                    get(storage);
                else
                    mark("=?", storage); //так как это const, должна быть инициализация
                currentExpression = expression(storage); // CONST *ident1* = *expr*; x = *expr*
                if(currentExpression->mode == ConstGen) { //опять же, так как это const
                    currentObject->value = currentExpression->a; //поля значений
                    currentObject->type = currentExpression->type; //поля типов
                }
                else
                    mark("Expression is not constant", storage);
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark(";?", storage); //после каждой переменной ;
            }
        } //размер const - 4, т.к. это всегда int
        if(storage->lastLexemeCode == typeLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //TYPE *ident1* = *type*; *ident2* = *type*; ...
                currentObject = createNewObject(TypGen, storage); //так же создаем объекты для каждого типа
                get(storage);
                if(storage->lastLexemeCode == eqlLexical)
                    get(storage);
                else
                    mark("=?", storage); //должно быть объявление типа
                currentObject->type = getType(storage); //получаем тип после
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark(";?", storage); //после каждой переменной ;
            }
        }
        if(storage->lastLexemeCode == varLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //*identList1* : *type; *identList2* : *type*; ...
                identListHead = identifiersList(VarGen, storage); //получаем список ВСЕХ идентификаторов через запятую, записываем
                                                    //в текущий scope, получаем указатель на первый записанный объект
                currentType = getType(storage); //получаем тип
                currentObject = identListHead;
                while(currentObject != storage->guard) { //для всех полученных объектов
                    varSize += currentType->size; //размер типа + к varSize
                    currentObject->type = currentType; //тип
                    currentObject->level = storage->currentLevel; //текущий уровень. для отличия переменных разных scope-ов
                    currentObject->value = -varSize; //todo смещение относительно базового адреса?
                    currentObject = currentObject->nextObject;
                }
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark(";?", storage); //после каждой переменной ;
            }
        }
        if((storage->lastLexemeCode >= constLexical) && (storage->lastLexemeCode <= varLexical))
            mark("Wrong declarations order?", storage); //потому что const->type->var
    }
    return varSize;

}

int parametersBlockAnalyzer(struct parameters *storage) {

    //чтение блока формальных параметров функции
    struct Object* listHead;
    struct Object* objectBuffer;
    struct Type* currentType;
    int typeSize;
    int parametersSize = 0;
    if(storage->lastLexemeCode == varLexical) { //[VAR] ??? : ???
        get(storage);
        listHead = identifiersList(ParGen, storage);
    }
    else
        listHead = identifiersList(VarGen, storage); //если VAR - VarGen, - параметр-значение. Иначе - ссылка - ParGen
    if(storage->lastLexemeCode == identLexical) { //ident - тип переменных
        objectBuffer = findObject(storage); //находим тип в существующих
        get(storage);
        if(objectBuffer->class == TypGen)
            currentType = objectBuffer->type; //если тип существует - присваиваем
        else {
            mark("Identifier is not a type?", storage);
            currentType = storage->intType; //иначе - ошибка. присваеваем int
        }
    }
    else {
        mark("Identifier type?", storage);
        currentType = storage->intType; //если тип не указан - присваиваем int
    }
    if(listHead->class == VarGen) {
        typeSize = currentType->size; //если размер отличается от допустимого (4) - ошибка, но все равно присваиваем
        if(currentType->type >= ArrayGen)
            mark("Incorrect type of var parameters", storage); //параметром var может быть только int либо bool
    }
    else
        typeSize = wordSize; //если не var - то int или bool. размер = 4
    objectBuffer = listHead;
    while(objectBuffer != storage->guard) { //инициализируем тип для всех новых объектов. считаем их размер
        objectBuffer->type = currentType;
        parametersSize += typeSize;
        objectBuffer = objectBuffer->nextObject;
    }
    return parametersSize;

}

void procedureAnalyzer(struct parameters *storage) {

    //анализ процедур. PROCEDURE *ident*(*FormalParameters*) *ProcBody*
    struct Object* currentProcedure;
    struct Object* objectBuffer;
    char procedureName[identLength];
    int currentBlockSize, parametersSize;
    int marksize = 8; //todo
    get(storage);
    if(storage->lastLexemeCode == identLexical) { //PROCEDURE *ident*
        strcpy(procedureName, storage->lastLexeme); //получили идент. процедуры
        currentProcedure = createNewObject(ProcGen, storage); //создание нового объекта для процедуры в текущем scope
        get(storage);
        parametersSize = marksize; //todo ??
        IncLevel(1, storage); //todo level for proc
        openScope(storage); //открываем scope для новой процедуры
        currentProcedure->value = -1; //todo why???
        if(storage->lastLexemeCode == lparenLexical) { //параметры функции (?? : ??; ?? : ??; ...)
            get(storage);
            if(storage->lastLexemeCode == rparenLexical) //если ()
                get(storage);
            else {
                parametersSize += parametersBlockAnalyzer(storage); //чтение ident1, ident2, .. : type
                while(storage->lastLexemeCode == semicolonLexical) { //между 2 и > должно быть ;
                    get(storage);
                    parametersSize += parametersBlockAnalyzer(storage);
                }
                if(storage->lastLexemeCode == rparenLexical)
                    get(storage);
                else
                    mark(")?", storage);
            }
        }
        objectBuffer = storage->topScope->nextObject; //переходим к первому параметру
        currentBlockSize = parametersSize;
        while(objectBuffer != storage->guard) { //полю val каждого объекта присваиваем значение смещения относительно
                                          //кадра активации. смещение положительное
            objectBuffer->level = storage->currentLevel;
            if(objectBuffer->class == ParGen)
                currentBlockSize -= wordSize;
            else
                currentBlockSize -= objectBuffer->type->size; //инкремент. смещение каждую итерацию
            objectBuffer->value = currentBlockSize; //присваивание смещения
            objectBuffer = objectBuffer->nextObject;
        }
        currentProcedure->previousScope = storage->topScope->nextObject; //дескриптор процесса - его параметры ? todo
        if(storage->lastLexemeCode == semicolonLexical)
            get(storage);
        else
            mark(";?", storage); //после PROCEDURE ident() должно быть ;
        currentBlockSize = 0; //теперь отрицательные сдвиги
        declarations(storage, currentBlockSize); //лок. параметры функции
        while(storage->lastLexemeCode == procedureLexical) {
            procedureAnalyzer(storage);
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                mark(";?", storage);
        } //внутр. функции функции
        currentProcedure->value = storage->PC; //todo
        Enter(currentBlockSize, storage); //запись пролога процедуры
        if(storage->lastLexemeCode == beginLexical) { //тело процедуры BEGIN procbody END *procident*
            get(storage);
            statements(storage); //последовательность операторов
        }
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("END?", storage);
        if(storage->lastLexemeCode == identLexical) {
            if(strcmp(procedureName, storage->lastLexeme))
                mark("No match procedurt identifier", storage); //идент. в конце должен совпадать с идент. процедуры
            get(storage);
        }
        Return(parametersSize - marksize, storage); //запись эпилога процедуры
        closeScope(storage); //закрытие scope
        IncLevel(-1, storage); //-1 уровень вложенности
    }

}

struct Object* parameter(struct Object* formalParameter, struct parameters* storage) {

    //проверка параметра процедуры на корректность
    struct Item* parameterExpression;
    struct Object* nextFormalParameter;
    nextFormalParameter = formalParameter;
    parameterExpression = expression(storage); //получение выражения параметра
    if(parameterFormat(nextFormalParameter)) {
        Parameter(parameterExpression, nextFormalParameter->type, nextFormalParameter->class, storage); //todo
        nextFormalParameter = nextFormalParameter->nextObject; //переход к следующему параметру
    }
    else
        mark("Too many parameters", storage);
    return nextFormalParameter;

}

int parameterFormat(struct Object *object) {

    //проверка, является ли объект допустимым параметром (проверка только формата)
    //параметр - Par либо Var. значение val - положительное, так как положительное смещение
    return ((object->class == ParGen) || ((object->class == VarGen) && (object->value > 0)));

}

struct Item* getSingleParameter(struct parameters *storage) {

    //получение выражения для глоб. процедур. ( ВЫРАЖЕНИЕ )
    struct Item* currentParameter;
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

    //*оператор* {; *оператор*}
    //оператор: присваивание / вызов процедуры / If / While
    struct Object* parametersObject;
    struct Object* currentObject;
    struct Item* currentItem;
    struct Item* bufferItem;
    int jumpLevel;
    do {
        currentObject = storage->guard; //иниц. для того, чтобы было не null
        if(storage->lastLexemeCode < identLexical) {
            mark("statement?", storage);
            do {
                get(storage);
            } while(storage->lastLexemeCode < identLexical);
        } //идем по коду, пока не будем корректных символов
        if(storage->lastLexemeCode == identLexical) { //если слово - идентификатор
            currentObject = findObject(storage); //находим, существует ли он
            get(storage);
            currentItem = MakeItem(currentObject, storage); //создаем item для вычисления выражения
            currentItem = selector(currentItem, storage); //получаем значение по индексу/полю, если оно есть
            if(storage->lastLexemeCode == becomesLexical) { //x := y
                get(storage);
                bufferItem = expression(storage);
                Store(currentItem, bufferItem, storage); //запись y в x
            }
            else if(storage->lastLexemeCode == eqlLexical) {
                mark(":=?", storage);
                get(storage);
                bufferItem = expression(storage);
            } //если =, предполагаем, что пропущено :
            else if(currentItem->mode == ProcGen) { //если объект - процедура, то это ее вызов
                parametersObject = currentObject->previousScope; //получение формальных параметров процедуры. par - указатель на первый
                if(storage->lastLexemeCode == lparenLexical) { //procedure(params)
                    get(storage);
                    if(storage->lastLexemeCode == rparenLexical)
                        get(storage);
                    else { //чтение передаваемых параметров
                        do {
                            parametersObject = parameter(parametersObject, storage); //проверка и запись передаваемых параметров
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
                    mark("Forward procedure call", storage); //если вызов процедуры todo
                else if(!parameterFormat(parametersObject)) //если все параметры корректны - запись вызова процедуры
                    Call(currentItem, storage);
                else
                    mark("Too few parameters", storage); //иначе - ошибка
            }
            else if(currentItem->mode == SProcGen) { //если одна из объявленных в universe процедур: Read, Write, WriteHex, WriteLn
                if(currentObject->value <= 3) //для каждой процедуры свое значение: 1, 2, 3, 4. Для 1, 2, 3 есть аргументы,
                    bufferItem = getSingleParameter(storage); //для WriteLn - нет. здесь получаем аргумент
                IOCall(currentItem, bufferItem, storage); //запись вызова
            }
            else if(currentObject->class == TypGen)
                mark("Illegal type assignment?", storage); //если присваивание типа?
            else
                mark("Illegal statement?", storage);
        }
        else if(storage->lastLexemeCode == ifLexical) { //если цикл if
            get(storage);
            currentItem = expression(storage); //анализ выражения ( exp )
            CJump(currentItem, storage); //todo
            if(storage->lastLexemeCode == thenLexical)
                get(storage);
            else
                mark("THEN?", storage); //IF *exp* THEN
            statements(storage); //последовательность операторов
            jumpLevel = 0;
            while(storage->lastLexemeCode == elsifLexical) {
                get(storage);
                jumpLevel = FJump(jumpLevel, storage); //todo
                FixLink(currentItem->a, storage); //todo
                currentItem = expression(storage); //получение очередного условия перехода
                CJump(currentItem, storage); //todo
                if(storage->lastLexemeCode == thenLexical)
                    get(storage);
                else
                    mark("THEN?", storage); //ELSIF *exp* THEN
                statements(storage); //последовательность операторов
            }
            if(storage->lastLexemeCode == elseLexical) { //последнее условие
                get(storage);
                jumpLevel = FJump(jumpLevel, storage); //todo
                FixLink(currentItem->a, storage); //todo
                statements(storage);
            }
            else
                FixLink(currentItem->a, storage); //todo
            FixLink(jumpLevel, storage); //todo
            if(storage->lastLexemeCode == endLexical)
                get(storage);
            else
                mark("END?", storage); //IF *...* END !
        }
        else if(storage->lastLexemeCode == whileLexical) { //WHILE *cond* DO *statements* END
            get(storage);
            jumpLevel = storage->PC; //todo
            currentItem = expression(storage); //получаем условие
            CJump(currentItem, storage); //todo
            if(storage->lastLexemeCode == doLexical)
                get(storage);
            else
                mark("DO?", storage);
            statements(storage); //последовательность операторов
            BJump(jumpLevel, storage); //todo
            FixLink(currentItem->a, storage); //todo
            if(storage->lastLexemeCode == endLexical)
                get(storage);
            else
                mark("END?", storage);
        }
        if(storage->lastLexemeCode == semicolonLexical)
            get(storage);
        else if((storage->lastLexemeCode <= identLexical) || (storage->lastLexemeCode == ifLexical) || (storage->lastLexemeCode == whileLexical)) {
            mark(";?", storage); //если идент, if или while, но между ними нет ;
        }

    } while((storage->lastLexemeCode <= identLexical) || (storage->lastLexemeCode == ifLexical) || (storage->lastLexemeCode == whileLexical));

}

void closeScope(struct parameters* storage) {

    storage->topScope = storage->topScope->previousScope; //возврат к предыдущему scope

}

void module(struct parameters* storage) {

    char moduleId[identLength] = "\0"; //название модуля
    int varSize = 0; //размер памяти для переменных и процедур (до begin)
    signal("Compilation begins.", storage);
    if(storage->lastLexemeCode == moduleLexical) { //модуль начинается с MODULE
        get(storage);
        openScope(storage); //открываем universe
        if(storage->lastLexemeCode == identLexical) { //MODULE *ident*
            strcpy(moduleId, storage->lastLexeme);
            char recordingString[identLength + 17] = "Compiles module "; //строка для вывода signal.
            strcat(recordingString, moduleId);
            strcat(recordingString, ".");
            signal(recordingString, storage);
            get(storage);
        }
        else {
            mark("Module ident?", storage);
        }
        if(storage->lastLexemeCode == semicolonLexical) //MODULE *ident*;
            get(storage);
        else
            mark(";?", storage);
        varSize = declarations(storage, varSize); //читаем объявления CONST, TYPE, VAR. varSize += их размер +++
        while(storage->lastLexemeCode == procedureLexical) { //читаем процедуры
            procedureAnalyzer(storage);
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                mark(";?", storage); //PROCEDURE procbody; !
        }
        Header(varSize, storage); //пролог. запись точки входа и указателя на вершину стека
        if(storage->lastLexemeCode == beginLexical) { //начало модуля
            get(storage);
            statements(storage); //анализ последовательности операторов
        }
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("END?", storage); //конец модуля
        if(storage->lastLexemeCode == identLexical) {
            if(strcmp(moduleId, storage->lastLexeme)) {
                mark("Wrong module identifier", storage);
            }
            get(storage);
        }
        else
            mark("Module identifier?", storage);
        if(storage->lastLexemeCode != periodLexical)
            mark(".?", storage); //MODULE *ident*.
        closeScope(storage); //закрытие universe
        if(!storage->error) { //если не было ошибок - генерируем код
            Close(storage); //эпилог. возврат вершины стека
            signal("Code generated.", storage);
        }
    }
    else {
        mark("Module?", storage);
    }

}

void compile(char *sourceCode) {

    struct parameters* storage =
            (struct parameters*)malloc(sizeof(struct parameters));
    if(init(storage, sourceCode) != 0) {
        return;
    } //важно, потому что могут быть ошибки обращения к памяти
    get(storage); //получение первого символа
    module(storage); //анализ кода, компиляция, запись кода
    signal("Compilation finished.", storage);
    decode(storage);

}