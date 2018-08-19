#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "scanner.h"
#include "codeGenerator.h"

void openScope(struct parameters*);

void signal(char msg[], struct parameters* storage) {
    fprintf(storage->reportFile, "%s\r\n", msg);
    printf("%s\n", msg);
}

void enter(int class, int n, char name[], struct Type* type, struct parameters* storage) {

    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));

    obj->class = class;
    obj->val = n;
    strcpy(obj->name, name);
    obj->type = type;
    obj->dsc = NULL;
    obj->next = storage->topScope->next;
    storage->topScope->next = obj;

}

void EnterKW(int sym, char name[], struct keyLex* keyTab[], int* i) {

    keyTab[*i]->sym = sym;
    strcpy(keyTab[*i]->id, name);
    (*i)++;

}

void InitLexical(struct parameters* storage) {

    for(int i = 0; i < 34; i++) {
        storage->keyTab[i] = (struct keyLex*)malloc(sizeof(struct keyLex));
    }

    int i = 0;
    EnterKW(nullLexical, "BY", storage->keyTab, &i);
    EnterKW(doLexical, "DO", storage->keyTab, &i);
    EnterKW(ifLexical, "IF", storage->keyTab, &i);
    EnterKW(nullLexical, "IN", storage->keyTab, &i);
    EnterKW(nullLexical, "IS", storage->keyTab, &i);
    EnterKW(ofLexical, "OF", storage->keyTab, &i);
    EnterKW(orLexical, "OR", storage->keyTab, &i);
    EnterKW(nullLexical, "TO", storage->keyTab, &i);
    EnterKW(endLexical, "END", storage->keyTab, &i);
    EnterKW(nullLexical, "FOR", storage->keyTab, &i);
    EnterKW(modLexical, "MOD", storage->keyTab, &i);
    EnterKW(nullLexical, "NIL", storage->keyTab, &i);
    EnterKW(varLexical, "VAR", storage->keyTab, &i);
    EnterKW(nullLexical, "CASE", storage->keyTab, &i);
    EnterKW(elseLexical, "ELSE", storage->keyTab, &i);
    EnterKW(nullLexical, "EXIT", storage->keyTab, &i);
    EnterKW(thenLexical, "THEN", storage->keyTab, &i);
    EnterKW(typeLexical, "TYPE", storage->keyTab, &i);
    EnterKW(nullLexical, "WITH", storage->keyTab, &i);
    EnterKW(arrayLexical, "ARRAY", storage->keyTab, &i);
    EnterKW(beginLexical, "BEGIN", storage->keyTab, &i);
    EnterKW(constLexical, "CONST", storage->keyTab, &i);
    EnterKW(elsifLexical, "ELSIF", storage->keyTab, &i);
    EnterKW(nullLexical, "IMPORT", storage->keyTab, &i);
    EnterKW(nullLexical, "UNTIL", storage->keyTab, &i);
    EnterKW(whileLexical, "WHILE", storage->keyTab, &i);
    EnterKW(recordLexical, "RECORD", storage->keyTab, &i);
    EnterKW(nullLexical, "REPEAT", storage->keyTab, &i);
    EnterKW(nullLexical, "RETURN", storage->keyTab, &i);
    EnterKW(nullLexical, "POINTER", storage->keyTab, &i);
    EnterKW(procedureLexical, "PROCEDURE", storage->keyTab, &i);
    EnterKW(divLexical, "DIV", storage->keyTab, &i);
    EnterKW(nullLexical, "LOOP", storage->keyTab, &i);
    EnterKW(moduleLexical, "MODULE", storage->keyTab, &i);

}

void initTypes(struct parameters* storage) {

    storage->boolType = (struct Type*)malloc(sizeof(struct Type));
    storage->boolType->form = BooleanGen;
    storage->boolType->size = 4;

    storage->intType = (struct Type*)malloc(sizeof(struct Type));
    storage->intType->form = IntegerGen;
    storage->intType->size = 4;

}

void initScopes(struct parameters* storage) {

    //в dsc хранится предыдущий scope
    //в next хранятся объекты текущего scope
    storage->topScope = (struct Object*)malloc(sizeof(struct Object));
    storage->guard = (struct Object*)malloc(sizeof(struct Object));
    storage->universe = (struct Object*)malloc(sizeof(struct Object));

    storage->guard->class = VarGen;
    storage->guard->type = storage->intType;
    storage->guard->val = 0;

    storage->topScope = NULL;
    openScope(storage);

    enter(TypGen, 1, "BOOLEAN", storage->boolType, storage);
    enter(TypGen, 2, "INTEGER", storage->intType, storage);
    enter(ConstGen, 1, "TRUE", storage->boolType, storage);
    enter(ConstGen, 0, "FALSE", storage->boolType, storage);
    enter(SProcGen, 1, "Read", NULL, storage);
    enter(SProcGen, 2, "Write", NULL, storage);
    enter(SProcGen, 3, "WriteHex", NULL, storage);
    enter(SProcGen, 4, "WriteLn", NULL, storage);

    storage->universe = storage->topScope;

}

int init(struct parameters* storage, char* sourceCode) {

    storage->sourceCode = sourceCode;
    storage->lastPosition = 0;
    strcpy(storage->lastLexeme, "\0");
    storage->lastLexemeCode = -1;
    storage->lastLexemeValue = -1;
    storage->error = 0;
    storage->errpos = -1;

    storage->reportFile = NULL;
    if((storage->reportFile = fopen("report.txt", "wb")) == NULL) {
        printf("Opening report.txt file error!\n");
        return -1;
    }

    InitLexical(storage);

    storage->linesCounter = 0;

    storage->curlev = 0;
    storage->pc = 0;
    storage->cno = 0;
    memset(storage->regs, 0, sizeof(int) * 16);

    initTypes(storage);

    initScopes(storage);

    for(int i = 0; i < maxCode; i++)
        storage->code[i] = 0;

    storage->cno = 0;

    storage->entry = 0;

    return 0;

}


struct Object* createNewObject(int class, struct parameters* storage) {

    //создание нового объекта
    struct Object* newObject = (struct Object*)malloc(sizeof(struct Object));
    struct Object* buffer;
    buffer = storage->topScope;
    strcpy(storage->guard->name, storage->lastLexeme);
    while(strcmp(buffer->next->name, storage->lastLexeme))
        buffer = buffer->next;
    if(buffer->next == storage->guard) { //если дошли до guard - такого объекта в этом scope нет
        strcpy(newObject->name, storage->lastLexeme);
        newObject->class = class;
        newObject->next = storage->guard;
        buffer->next = newObject;
    }
    else { //иначе - ошибка
        newObject = buffer->next;
        mark("Declared already", storage);
    }
    strcpy(storage->guard->name, "\0"); //"обнуляем" guard
    return newObject;

}

struct Object* find(struct parameters* storage) {

    struct Object* bufferHead;
    struct Object* buffer;
    struct Object* object = (struct Object*)malloc(sizeof(struct Object));
    bufferHead = storage->topScope;
    strcpy(storage->guard->name, storage->lastLexeme);
    do {
        buffer = bufferHead->next;
        while(strcmp(buffer->name, storage->lastLexeme))
            buffer = buffer->next;
        if(buffer != storage->guard)
            object = buffer; //если проверили scope и нашли объект
        else if(bufferHead == storage->universe) { //если не нашли и мы уже на внешнем scope
            object = buffer;
            mark("Not declared", storage);
            buffer = NULL; //для выхода из цикла
        }
        else
            bufferHead = bufferHead->dsc; //иначе идем на уровень ниже
    } while(buffer == storage->guard);
    strcpy(storage->guard->name, "\0"); //"обнуляем" guard
    return object;

}

struct Item* selector(struct Item* item, struct parameters* storage) { //передается уже инициализированный элемент

    struct Item* newItem = item; //возвращаемый item
    struct Item* indexExpression; //для выражения индекса массива
    struct Object* fieldObject; //для поиска поля записи
    //если нет обращения к элементам, то это просто переменная
    while((storage->lastLexemeCode == lbrakLexical) || (storage->lastLexemeCode == periodLexical)) { // [ либо .
        if(storage->lastLexemeCode == lbrakLexical) {
            get(storage);
            indexExpression = expression(storage); //x[y]. получаем y
            if(newItem->type->form == ArrayGen)
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
                if(newItem->type->form == RecordGen) { //если тип x - запись
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

    //*ident*, *number*
    struct Item* item;
    struct Object* object;
    if(storage->lastLexemeCode < lparenLexical) {
        mark("Identifier? Number?", storage);
        do {
            get(storage);
        } while(storage->lastLexemeCode < lparenLexical);
    } //проверяем допустимость символов
    if(storage->lastLexemeCode == identLexical) {
        object = find(storage); //ищем объект в существующих
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
        Op1(item, notLexical, storage); //выражение со знаком -
    } //отрицание
    else {
        mark("Factor?", storage);
        item = MakeItem(storage->guard, storage); //возвращаем guard, если нет допустимой переменной
    }
    return item;

}

struct Item* term(struct parameters* storage) {

    //x * y, x DIV y, x MOD y, x & y
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

    //сумма, разность, |. учитывается знак перед первым аргументом (- или +)
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

    //либо переменная x, либо лог. выражение x ( = | # | < | <= | > | >= ) y
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

struct Type* Type(struct parameters* storage) {

    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));
    struct Object* first;
    struct Item* x;
    struct Type* tp;
    struct Type* type;
    type = storage->intType;
    //типы: array, record, intType, boolType, созданный тип
    if((storage->lastLexemeCode != identLexical) && (storage->lastLexemeCode < arrayLexical)) {
        mark("type?", storage);
        do {
            get(storage);
        } while((storage->lastLexemeCode != identLexical) && (storage->lastLexemeCode < arrayLexical));
    } //проверяем допустимые символы
    if(storage->lastLexemeCode == identLexical) { //если это объявленный тип
        obj = find(storage); //находим в существующих
        get(storage);
        if(obj->class == TypGen) //если класс найденного объекта - тип, то присваиваем
            type = obj->type;
        else
            mark("Type identifier?", storage);
    }
    else if(storage->lastLexemeCode == arrayLexical) { //ARRAY *expression* OF *type*
        get(storage);
        x = expression(storage);
        if((x->mode != ConstGen) || (x->a < 0)) //размер массива - ПОСТОЯННАЯ, значение - НЕ ОТРИЦАТЕЛЬНОЕ
            mark("Bad index", storage);
        if(storage->lastLexemeCode == ofLexical)
            get(storage);
        else
            mark("OF?", storage);
        type->form = ArrayGen;
        type->base = Type(storage); //баз. тип
        type->len = x->a; // "длина" массива
        type->size = type->len * type->base->size; //размер массива - длина * размер типа
    }
    else if(storage->lastLexemeCode == recordLexical) { //RECORD *ident1*, *ident2* : *type1* {; *identN* : *typeN*} END
        get(storage);
        type->form = RecordGen;
        type->size = 0;
        openScope(storage);
        while((storage->lastLexemeCode == semicolonLexical) || (storage->lastLexemeCode == identLexical)) { //чтение полей
            if(storage->lastLexemeCode == identLexical) {
                first = IdentList(FldGen, storage); //записываем объекты в scope
                tp = Type(storage); //получаем тип
                obj = first;
                while(obj != storage->guard) { //записываем тип для всех идентификаторов
                    obj->type = tp;
                    obj->val = type->size;
                    type->size += tp->size;
                    obj = obj->next;
                }
            }
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else if(storage->lastLexemeCode == identLexical)
                mark(";?", storage);
        }
        *(type->fields) = *(storage->topScope->next);
        //CloseScope(); //todo
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("END?", storage);
    } //todo test it
    else
        mark("ident type?", storage);
    return type;

}

struct Object* IdentList(int class, struct parameters* storage) {
    struct Object* first = (struct Object*)malloc(sizeof(struct Object));
    if(storage->lastLexemeCode == identLexical) {
        first = createNewObject(class, storage); //в first первый идентификатор
        get(storage);
        while(storage->lastLexemeCode == commaLexical) {
            get(storage);
            if(storage->lastLexemeCode == identLexical) {
                createNewObject(class, storage);
                get(storage);
            }
            else
                mark("ident?", storage);
        }
        if(storage->lastLexemeCode == colonLexical)
            get(storage);
        else
            mark(":?", storage);
    }
    return first;
}

void openScope(struct parameters* storage) {

    struct Object* newScope = (struct Object*)malloc(sizeof(struct Object));
    newScope->class = HeadGen; //первый элемент scope - head
    newScope->dsc = storage->topScope; //dsc - указатель на предыдущий scope
    newScope->next = storage->guard; //след. элемента нет
    storage->topScope = newScope; //новая вершина scopes

}

int declarations(struct parameters* storage, int argVarSize) { //возвращается новое значение varSize

    struct Item* currentExpression; //структура для анализа выражений
    struct Object* currentObject; //структура для рассматриваемого объекта
    struct Object* identListHead; //для множества идентификаторов. указатель на первый элемент
    struct Type* currentType; //тип переменной
    int varSize = argVarSize;
    if((storage->lastLexemeCode < constLexical) && (storage->lastLexemeCode != endLexical)) {
        mark("Declaration?", storage);
        do {
            get(storage);
        } while ((storage->lastLexemeCode < constLexical) && (storage->lastLexemeCode != endLexical));
    } //если не const, type, var, procedure, begin, module, end. останавливается на eof
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
                currentExpression = expression(storage); // CONST *ident1* = *expr*; x = *expr* todo
                if(currentExpression->mode == ConstGen) { //опять же, так как это const
                    currentObject->val = currentExpression->a; //поля значений
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
                currentObject->type = Type(storage); //получаем тип после = todo
                if(storage->lastLexemeCode == semicolonLexical)
                    get(storage);
                else
                    mark(";?", storage); //после каждой переменной ;
            }
        }
        if(storage->lastLexemeCode == varLexical) {
            get(storage);
            while(storage->lastLexemeCode == identLexical) { //*identList1* : *type; *identList2* : *type*; ...
                identListHead = IdentList(VarGen, storage); //получаем список ВСЕХ идентификаторов через запятую, записываем
                                                    //в текущий scope, получаем указатель на первый записанный объект todo
                currentType = Type(storage); //получаем тип
                currentObject = identListHead;
                while(currentObject != storage->guard) { //для всех полученных объектов
                    varSize += currentType->size; //размер типа + к varSize
                    currentObject->type = currentType; //тип
                    currentObject->lev = storage->curlev; //текущий уровень. для отличия переменных разных scope-ов
                    currentObject->val = -varSize; //todo смещение относительно базового адреса?
                    currentObject = currentObject->next;
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

int FPSection(int parblksize, struct parameters* storage) {

    struct Object* first = (struct Object*)malloc(sizeof(struct Object));
    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));
    struct Type* tp = (struct Type*)malloc(sizeof(struct Type));
    int parsize;
    int parblksizeRet = parblksize;

    if(storage->lastLexemeCode == varLexical) { //[VAR] ??? : ???
        get(storage);
        first = IdentList(ParGen, storage);
    }
    else
        first = IdentList(VarGen, storage);
    if(storage->lastLexemeCode == identLexical) {
        obj = find(storage);
        get(storage);
        if(obj->class == TypGen)
            tp = obj->type;
        else {
            mark("ident type?", storage);
            tp = storage->intType;
        }
    }
    else {
        mark("ident?", storage);
        tp = storage->intType;
    }
    if(first->class == VarGen) {
        parsize = tp->size;
        if(tp->form >= ArrayGen)
            mark("no struct params", storage);
    }
    else
        parsize = WordSize;
    obj = first;
    while(obj != storage->guard) {
        obj->type = tp;
        parblksizeRet += parsize;
        obj = obj->next;
    }

}

void ProcedureDecl(struct parameters* storage) {

    struct Object* proc = (struct Object*)malloc(sizeof(struct Object));
    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));
    char procid[identLength];
    int locblksize, parblksize;
    int marksize = 8;

    get(storage);
    if(storage->lastLexemeCode == identLexical) { //PROC ident
        strcpy(procid, storage->lastLexeme);
        proc = createNewObject(ProcGen, storage);
        get(storage);
        parblksize = marksize;
        IncLevel(1, storage);
        openScope(storage);
        proc->val = -1;
        if(storage->lastLexemeCode == lparenLexical) { //параметры функции (?? : ??; ?? : ??; ...)
            get(storage);
            if(storage->lastLexemeCode == rparenLexical)
                get(storage);
            else {
                FPSection(parblksize, storage);
                while(storage->lastLexemeCode == semicolonLexical) {
                    get(storage);
                    FPSection(parblksize, storage);
                }
                if(storage->lastLexemeCode == rparenLexical)
                    get(storage);
                else
                    mark(")?", storage);
            }
        }
        else if(storage->curlev == 1) {
            EnterCmd(procid, storage);
        }
        obj = storage->topScope->next;
        locblksize = parblksize;
        while(obj != storage->guard) {
            obj->lev = storage->curlev;
            if(obj->class == ParGen)
                locblksize -= WordSize;
            else {
                obj->val = locblksize;
                obj = obj->next;
            }
        }
        proc->dsc = storage->topScope->next; //хз че тут происходит...
        if(storage->lastLexemeCode == semicolonLexical)
            get(storage);
        else
            mark(";?", storage);
        locblksize = 0;
        declarations(storage, locblksize); //лок. параметры функции
        while(storage->lastLexemeCode == procedureLexical) {
            ProcedureDecl(storage);
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                mark(";?", storage);
        } //внутр. функции функции
        proc->val = PCGen;
        Enter(locblksize, storage);
        if(storage->lastLexemeCode == beginLexical) {
            get(storage);
            StatSequence(storage);
        }
        if(storage->lastLexemeCode == endLexical)
            get(storage);
        else
            mark("end?", storage);
        if(storage->lastLexemeCode == identLexical) {
            if(strcmp(procid, storage->lastLexeme))
                mark("no match", storage);
            get(storage);
        }
        Return(parblksize - marksize, storage);
        closeScope(storage);
        IncLevel(-1, storage);
    }

}

struct Object* parameter(struct Object* fp, struct parameters* storage) {

    struct Item* x;
    struct Object* newFp;
    newFp = fp;
    x = expression(storage);
    if(IsParam(newFp)) {
        Parameter(x, newFp->type, newFp->class, storage);
        newFp = newFp->next;
    }
    else
        mark("too many parameters", storage);
    return newFp;

}

int IsParam(struct Object* obj) {

    return ((obj->class == ParGen) || ((obj->class == VarGen) && (obj->val > 0)));

}

struct Item* param(struct parameters* storage) {

    struct Item* x = (struct Item*)malloc(sizeof(struct Item));
    if(storage->lastLexemeCode == lparenLexical)
        get(storage);
    else
        mark("(?", storage);
    x = expression(storage);
    if(storage->lastLexemeCode == rparenLexical)
        get(storage);
    else
        mark(")?", storage);
    return x;

}

void StatSequence(struct parameters* storage) {

    //*statement* {; *statement*}
    //statement: assigment/ ProcCall/ If/ While
    struct Object* par = (struct Object*)malloc(sizeof(struct Object));
    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));
    struct Item* x = (struct Item*)malloc(sizeof(struct Item));
    struct Item* y = (struct Item*)malloc(sizeof(struct Item));
    int L;

    do {
        obj = storage->guard;
        if(storage->lastLexemeCode < identLexical) {
            mark("statement?", storage);
            do {
                get(storage);
            } while(storage->lastLexemeCode < identLexical);
        }
        if(storage->lastLexemeCode == identLexical) {
            find(storage);
            get(storage);
            x = MakeItem(obj, storage);
            x = selector(x, storage);
            if(storage->lastLexemeCode == becomesLexical) { //x := y
                get(storage);
                y = expression(storage);
                Store(x, y, storage);
            }
            else if(storage->lastLexemeCode == eqlLexical) {
                mark(":=?", storage);
                get(storage);
                y = expression(storage);
            }
            else if(x->mode == ProcGen) {
                par = obj->dsc; //параметры вызываемой функции
                if(storage->lastLexemeCode == lparenLexical) {
                    get(storage);
                    if(storage->lastLexemeCode == rparenLexical)
                        get(storage);
                    else {
                        do {
                            par = parameter(par, storage);
                            if(storage->lastLexemeCode == commaLexical)
                                get(storage); //неск. параметров - запятые
                            else if(storage->lastLexemeCode == rparenLexical)
                                get(storage); //хз
                            else if(storage->lastLexemeCode < semicolonLexical)
                                mark(") or , ?", storage);
                        } while(!((storage->lastLexemeCode == rparenLexical) || (storage->lastLexemeCode >= semicolonLexical)));
                    }
                }
                if(obj->val < 0)
                    mark("forward call", storage);
                else if(!IsParam(par))
                    Call(x, storage);
                else
                    mark("too few parameters", storage);
            }
            else if(x->mode == SProcGen) {
                if(obj->val <= 3) {}
                    y = param(storage);
                IOCall(x, y, storage);
            }
            else if(obj->class == TypGen)
                mark("illegal assignment?", storage);
            else
                mark("statement?", storage);
        }
        else if(storage->lastLexemeCode == ifLexical) {
            get(storage);
            x = expression(storage);
            CJump(x, storage);
            if(storage->lastLexemeCode == thenLexical)
                get(storage);
            else
                mark("THEN?", storage);
            StatSequence(storage);
            L = 0;
            while(storage->lastLexemeCode == elsifLexical) {
                get(storage);
                FJump(L, storage);
                FixLink(x->a, storage);
                x = expression(storage);
                CJump(x, storage);
                if(storage->lastLexemeCode == thenLexical)
                    get(storage);
                else
                    mark("THEN?", storage);
                StatSequence(storage);
            }
            if(storage->lastLexemeCode == elseLexical) {
                get(storage);
                FJump(L, storage);
                FixLink(x->a, storage);
                StatSequence(storage);
            }
            else
                FixLink(x->a, storage);
            FixLink(L, storage);
            if(storage->lastLexemeCode == endLexical)
                get(storage);
            else
                mark("END?", storage);
        }
        else if(storage->lastLexemeCode == whileLexical) {
            get(storage);
            L = storage->pc;
            x = expression(storage);
            CJump(x, storage);
            if(storage->lastLexemeCode == doLexical)
                get(storage);
            else
                mark("DO?", storage);
            StatSequence(storage);
            BJump(L, storage);
            FixLink(x->a, storage);
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

    storage->topScope = storage->topScope->dsc; //возврат к предыдущему scope

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
        varSize = declarations(storage, varSize); //читаем объявления CONST, TYPE, VAR. varSize += их размер
        while(storage->lastLexemeCode == procedureLexical) { //читаем процедуры
            ProcedureDecl(storage);
            if(storage->lastLexemeCode == semicolonLexical)
                get(storage);
            else
                mark(";?", storage); //PROCEDURE procbody; !
        }
        Header(varSize, storage); //todo
        if(storage->lastLexemeCode == beginLexical) { //начало модуля
            get(storage);
            StatSequence(storage); //анализ последовательности операторов
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
            Close(storage);
            signal("Code generated.", storage);
        }
    }
    else {
        mark("Module?", storage);
    }

}

void Compile(char* sourceCode) {

    struct parameters* storage =
            (struct parameters*)malloc(sizeof(struct parameters));
    if(init(storage, sourceCode) != 0) {
        return;
    } //важно, потому что могут быть ошибки обращения к памяти
    get(storage); //получение первого символа
    module(storage); //анализ кода
    signal("Compilation finished.", storage);
    decode(storage);

}