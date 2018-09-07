#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "scanner.h"
#include "codeGenerator.h"

void testRange(int x, struct parameters *storage) {

    //проверка на переполнение
    if((x >= 0x20000) || (x < -0x20000)) //131072
        mark("Value is too large", storage);

}

void put(int command, int a, int b, int c, struct parameters *storage) {

    //запись сгенерированной команды
    //первые 5 бит - команда, след. 4 бита - a, след. 4 бита - b, след. 18 - c
    if(command >= 32)
        command -= 64; //если команда больше 5 бит. в декодере учитывается знак
    storage->code[storage->PC] = (((command << 4 | a) << 4 | b) << 18) | (c & 0x3FFFF);
    storage->PC++;

}

int getReg(struct parameters *storage) {

    //получение свободного регистра
    int newReg = 0;
    while((newReg < FPGen) && (storage->registers[newReg] == 1))
        newReg++;
    storage->registers[newReg] = 1;
    return newReg;

}

void load(struct item* x, struct parameters* storage) {

    //загрузка в регистр
    if(x->mode == VarGen) { //загрузка переменной
        int r = getReg(storage); //получение регистра
        if(x->level == 0)
            x->a -= storage->PC * 4; //если это глобальные переменные, то увеличение сдвига относительно базового адреса
        put(LDWGen, r, x->storage, x->a, storage); //загрузка в регистр значения переменной
        storage->registers[x->storage] = 0;
        x->storage = r; //обнуление регистра, в кот. хранился адрес переменной
    }
    else if(x->mode == ConstGen) { //загрузка константы
        testRange(x->a, storage); //проверка на переполнение
        x->storage = getReg(storage); //получение регистра
        put(MOVIGen, x->storage, 0, x->a, storage); //загрузка константы в регистр
    }
    x->mode = RegGen; //mode - загружен в регистр

}

void arrayElem(struct item *arrayVar, struct item *index, struct parameters *storage) {

    //получение элемента массива
    if(index->classType != storage->intType)
        mark("Index is not an integer", storage); //индекс - только int
    if(index->mode == ConstGen) { //если индекс - константа
        if((index->a < 0) || (index->a >= arrayVar->classType->length))
            mark("Bad index", storage);
        arrayVar->a += index->a * arrayVar->classType->baseType->size; //смещение относительно адреса массива
    }
    else { //если индекс - переменная
        if(index->mode != RegGen)
            load(index, storage); //если индекс не хранится в регистре - загрузка
        put(CHKIGen, index->storage, 0, arrayVar->classType->length, storage); //обнуление индекса, если он больше допустимого
        put(MULIGen, index->storage, index->storage, arrayVar->classType->baseType->size, storage); //получение смещения
        put(ADDGen, index->storage, arrayVar->storage, index->storage, storage); //добавление адреса к смещению
        storage->registers[arrayVar->storage] = 0;
        arrayVar->storage = index->storage; //обнуление регистра элемента
    }
    arrayVar->classType = arrayVar->classType->baseType; //типа элемента - базовый тип массива

}

struct object* findField(struct object *list, struct parameters *storage) {

    //поиск поля записи в scope записи
    strcpy(storage->guard->name, storage->lastLexeme);
    while(strcmp(list->name, storage->lastLexeme))
        list = list->nextObject;
    return list; //возврат либо поля, либо guard

}

void getField(struct item *record, struct object *field) {

    //инициализация поля записи
    record->a += field->value; //добавление смещения поля
    record->classType = field->classType; //присваивание типа поля

}

struct item* makeItem(struct object *argObject, struct parameters *storage) {

    //создание item из object
    struct item* newItem = (struct item*)malloc(sizeof(struct item));
    if(newItem == NULL) {
        mark("Memory allocation error in function makeItem", storage);
        return storage->emptyItem;
    }
    newItem->mode = argObject->class;
    newItem->classType = argObject->classType;
    newItem->level = argObject->level;
    newItem->a = argObject->value;
    newItem->b = 0;
    if(argObject->level == 0)
        newItem->storage = PCGen; //если глобальная переменная
    else if(argObject->level == storage->currentLevel)
        newItem->storage = FPGen; //если локальная - указывает на базовый адрес процедуры
    else
        newItem->storage = 0;
    if(argObject->class == ParGen) { //если параметр процедуры
        int reg = getReg(storage); //получение регистра для его хранения
        put(LDWGen, reg, newItem->storage, newItem->a, storage); //загрузка в регистр параметра: баз. адрес + смещение
        newItem->mode = VarGen;
        newItem->storage = reg;
        newItem->a = 0; //значение хранится в регистре. смещение - 0
    }
    return newItem;

}

struct item* makeConstItem(struct type *typ, int val, struct parameters *storage) {

    //создание item-константы без загрузки в регистр
    struct item* newItem = (struct item*)malloc(sizeof(struct item));
    if(newItem == NULL) {
        mark("Memory allocation error in function makeConstItem", storage);
        return storage->emptyItem;
    }
    initItem(newItem, storage);
    newItem->mode = ConstGen;
    newItem->classType = typ;
    newItem->a = val;
    return newItem;

}

int negated(int cond) {

    //для команд переходов. если число четное - возврат нечетного и наоборот
    if(cond % 2)
        return cond - 1;
    else
        return cond + 1;

}

void loadBool(struct item* x, struct parameters* storage) {

    //load с проверкой на тип boolean
    //атрибуты cond: = 0; != 1; < 2; >= 3; <= 4; > 5
    if(x->classType->classType != BooleanGen)
        mark("Not a boolean type", storage);
    load(x, storage);
    x->mode = CondGen; //"режим" - логическое выражение
    x->a = 0; //адрес незавершенной команды перехода
    x->b = 0;
    x->c = 1; //по умолчанию для выражения атрубут !=

}

void putJump(int command, int disp, struct parameters *storage) {

    //запись команд перехода/сравнения
    //первые 5 бит - команда, след. 26 - смещение
    storage->code[storage->PC] = ((command - 0x40) << 26) | (disp & 0x3FFFFFF);
    storage->PC++;


}

void fix(int commandIndex, int jumpAddress, struct parameters* storage) {

    //запись адреса перехода в команду
    storage->code[commandIndex] = (storage->code[commandIndex] & 0xFFC00000) | (jumpAddress & 0x3FFFFF);

}

void fixLink(int jumpEntry, struct parameters *storage) {

    //запись адреса перехода для всех ответвлений
    int nextJump;
    while(jumpEntry != 0) {
        nextJump = storage->code[jumpEntry] & 0x3FFFF; //следующее ответвление
        fix(jumpEntry, storage->PC - jumpEntry, storage);
        jumpEntry = nextJump;
    }

}

void singleGenerate(int op, struct item *x, struct parameters *storage) {

    //генерация кода для выражений с одним аргументом. -x, ~x, x &, x |
    if(op == minusLexical) { //если -x
        if(x->classType->classType != IntegerGen)
            mark("Type must be integer", storage); //отрицательный только int
        else if(x->mode == ConstGen)
            x->a = -(x->a); //если константа - просто отрицательное значение
        else {
            if(x->mode == VarGen)
                load(x, storage); //если не загружена переменная - загрузка в регистр
            put(MVNGen, x->storage, 0, x->storage, storage); //загрузка с реверс. знаком
        }
    }
    else if(op == notLexical) { //если ~x
        if(x->mode != CondGen)
            loadBool(x, storage); //если не загружено в регист - загрузка
        x->c = negated(x->c); //= меняется на != и наоборот
        int t;
        t = x->a;
        x->a = x->b;
        x->b = t; //перестваление списков F- и T- переходов
    }
    else if(op == andLexical) { //x &
        if(x->mode != CondGen)
            loadBool(x, storage); //загрузка в регистр
        putJump(BEQGen + negated(x->c), x->a, storage); //переход дальше, если аргумент 1
        storage->registers[x->storage] = 0;
        x->a = storage->PC - 1;
        fixLink(x->b, storage); //запись адреса перехода
        x->b = 0;
    }
    else if(op == orLexical) { //x |
        if(x->mode != CondGen)
            loadBool(x, storage); //загрузка в регистр
        putJump(BEQGen + x->c, x->b, storage); //переход дальше, если аргумент 0
        storage->registers[x->storage] = 0;
        x->b = storage->PC - 1;
        fixLink(x->a, storage); //запись адреса перехода
        x->a = 0;
    }

}

void putArith(int cd, struct item *x, struct item *y, struct parameters *storage) {

    if(x->mode != RegGen)
        load(x, storage); //x - всегда в регистре
    if(y->mode == ConstGen) {
        testRange(y->a, storage);
        put(cd + 16, x->storage, x->storage, y->a, storage); //если y - константа, команда с I (E: MOVI)
    }
    else {
        if(y->mode != RegGen)
            load(y, storage); //иначе, y - в регистре
        put(cd, x->storage, x->storage, y->storage, storage);
        storage->registers[y->storage] = 0; //y освобождается
    }

}

int merged(int rightExp, int leftExp, struct parameters* storage) {

    //соединение логических подвыражений
    int rightBuff, secondBuff;
    if(rightExp != 0) { //если правое выражение есть, анализируем его
        rightBuff = rightExp;
        do {
            secondBuff = storage->code[rightBuff] & 0x3FFFF;
            if(secondBuff != 0)
                rightBuff = secondBuff;
        } while(secondBuff != 0);
        storage->code[rightBuff] = storage->code[rightBuff] + leftExp;
        return rightExp;
    }
    else
        return leftExp;

}

void termGenerate(int op, struct item *x, struct item *y, struct parameters *storage) {

    //генерирование кода для выражения x op y;
    //должны быть одинаковые типы!
    if((x->classType->classType == IntegerGen) && (y->classType->classType == IntegerGen)) {
        if((x->mode == ConstGen) && (y->mode == ConstGen)) { //две константы. в регистрах не хранятся
            if(op == plusLexical)
                x->a += y->a;
            else if(op == minusLexical)
                x->a -= y->a;
            else if(op == timesLexical)
                x->a *= y->a;
            else if(op == divLexical)
                x->a /= y->a;
            else if(op == modLexical)
                x->a = x->a % y->a;
            else
                mark("Bad expression operator", storage);
        }
        else { //если не константы - хранится в регистарах
            if(op == plusLexical)
                putArith(ADDGen, x, y, storage);
            else if(op == minusLexical)
                putArith(SUBGen, x, y, storage);
            else if(op == timesLexical)
                putArith(MULGen, x, y, storage);
            else if(op == divLexical)
                putArith(DIVGen, x, y, storage);
            else if(op == modLexical)
                putArith(MODGen, x, y, storage);
            else
                mark("Bad expression operator", storage);
        }
    }
    else if((x->classType->classType == BooleanGen) && (y->classType->classType == BooleanGen)) {
        if(y->mode != CondGen)
            loadBool(y, storage); //второй аргумент - в регистр
        if(op == orLexical) {
            x->a = y->a;
            x->b = merged(y->b, x->b, storage); //соединение логических подвыражений
            x->c = y->c;
        }
        else if(op == andLexical) {
            x->a = merged(y->a, x->a, storage); //соединение логических подвыражений
            x->b = y->b;
            x->c = y->c;
        }
    }
    else
        mark("Bad types", storage);

}

void relation(int op, struct item *x, struct item *y, struct parameters *storage) {

    //генерирование кода для выражения сравнения
    if((x->classType->classType != IntegerGen) || (y->classType->classType != IntegerGen))
        mark("Must be integer type", storage); //аргументы - только int
    else {
        putArith(CMPGen, x, y, storage); //команда сравнения
        x->c = op - eqlLexical; //код сравнения (есть в loadBool)
        storage->registers[y->storage] = 0;
    }
    x->mode = CondGen;
    x->classType = storage->boolType; //результат сравнения - boolean
    x->a = 0;
    x->b = 0;

}

void changeLevel(int n, struct parameters *storage) {

    //изменение уровня вложенности scope
    storage->currentLevel += n;

}

void prologue(int size, struct parameters *storage) {

    //команды пролога. сохранение адреса возврата
    put(PSHGen, LNKGen, SPGen, 4, storage);
    put(PSHGen, FPGen, SPGen, 4, storage);
    put(MOVGen, FPGen, 0, SPGen, storage);
    put(SUBIGen, SPGen, SPGen, size, storage);

}

void epilogue(int size, struct parameters *storage) {

    //команды эпилога. возврат стека
    put(MOVGen, SPGen, 0, FPGen, storage);
    put(POPGen, FPGen, SPGen, 4, storage);
    put(POPGen, LNKGen, SPGen, size + 4, storage);
    putJump(RETGen, LNKGen, storage);

}

void store(struct item *x, struct item *y, struct parameters *storage) {

    //присваивание
    if((x->classType->classType == BooleanGen || x->classType->classType == IntegerGen) && (x->classType->classType == y->classType->classType)) {
        if(y->mode == CondGen) { //присваивание результата сравнения. x := y
            put(BEQGen + negated(y->c), y->storage, 0, y->a, storage); //переход по лог. операции
            storage->registers[y->storage] = 0;
            y->a = storage->PC - 1; //переход, если истинно
            fixLink(y->b, storage); //запись адреса перехода, если не истинно
            y->storage = getReg(storage);
            put(MOVIGen, y->storage, 0, 1, storage); //загрузка истинного значения
            putJump(BRGen, 2, storage); //переход
            fixLink(y->a, storage);
            put(MOVIGen, y->storage, 0, 0, storage); //загрузка ложного значения
        }
        else if(y->mode != RegGen)
            load(y, storage); //если y не в регистре - в регистр
        if(x->mode == VarGen) {
            if(x->level == 0)
                x->a = x->a - (storage->PC) * 4;
            put(STWGen, y->storage, x->storage, x->a, storage); //если x переменная - сохранение в памяти
        }
        else
            mark("Illegal assignment", storage);
        storage->registers[x->storage] = 0;
        storage->registers[y->storage] = 0; //освобождение регистров, так как переменная уже в памяти
    }
    else
        mark("Incompatible assignment", storage);

}

void procedureCall(struct item *x, struct parameters *storage) {

    //вызов процедуры
    putJump(BSRGen, x->a - storage->PC, storage);

}

void globalCall(struct item *x, struct item *y, struct parameters *storage) {

    //вызов глобальных процедур
    struct item* z = (struct item*)malloc(sizeof(struct item));
    if(z == NULL) {
        mark("Memory allocation error in function globalCall", storage);
        return;
    }
    initItem(z, storage);
    if(x->a < 4) {
        if(y->classType->classType != IntegerGen)
            mark("Argument must be integer", storage);
    } //read, write, writehex - аргумент integer
    if(x->a == 1) { //read
        z->storage = getReg(storage);
        z->mode = RegGen;
        z->classType = storage->intType;
        put(RDGen, z->storage, 0, 0, storage);
        store(y, z, storage); //в регистр значение аргумента
    }
    else if(x->a == 2) { //write
        load(y, storage);
        put(WRDGen, 0, 0, y->storage, storage);
        storage->registers[y->storage] = 0; //печать числа
    }
    else if(x->a == 3) { //writehex
        load(y, storage);
        put(WRHGen, 0, 0, y->storage, storage);
        storage->registers[y->storage] = 0; //печать числа в формате 0x
    }
    else //writeln
        put(WRLGen, 0, 0, 0, storage); //печать \n
    if(z != NULL)
        free(z);

}

void falseJump(struct item *x, struct parameters *storage) {

    //переход, если условие не выполняется
    if(x->classType->classType == BooleanGen) {
        if(x->mode != CondGen)
            loadBool(x, storage); //загрузка в регистр
        putJump(BEQGen + negated(x->c), x->a, storage); //переход
        storage->registers[x->storage] = 0;
        fixLink(x->b, storage); //запись адреса перехода
        x->a = storage->PC - 1;
    }
    else {
        mark("Not boolean type", storage);
        x->a = storage->PC;
    }

}

void whileJump(int retAddress, struct parameters *storage) {

    //безусловный переход в конце цикла while
    putJump(BRGen, retAddress - storage->PC, storage);

}

int elseJump(int retAddress, struct parameters *storage) {

    //переход для elsif/else
    int newAddress;
    putJump(BRGen, retAddress, storage);
    newAddress = storage->PC - 1;
    return newAddress;

}

void codeHeader(int size, struct parameters *storage) {

    //запись пролога программы
    storage->entryAddress = storage->PC; //сохранение точки входа
    put(MOVIGen, SPGen, 0, 1024 - size, storage); //вершина стека = память машины - память глоб. переменных
    put(PSHGen, LNKGen, SPGen, 4, storage); //сохранение адреса вершины стека

}

void codeEnding(struct parameters *storage) {

    //конец кода.
    put(POPGen, LNKGen, SPGen, 4, storage);
    putJump(RETGen, LNKGen, storage);

}

void decode(struct parameters* storage) {

    //декордер, запись кода в файл
    FILE* outputFile = NULL;
    printf("Recording compiled code...\n");
    if((outputFile = fopen("output.txt", "wb")) == NULL) {
        printf("Opening output file error!\n");
        fprintf(storage->reportFile, "Opening output file error!\r\n");
        return;
    } //открытие output.txt
    char mnemo[64][6];
    strcpy(mnemo[MOVGen], "MOV ");
    strcpy(mnemo[MVNGen], "MVN ");
    strcpy(mnemo[ADDGen], "ADD ");
    strcpy(mnemo[SUBGen], "SUB ");
    strcpy(mnemo[MULGen], "MUL ");
    strcpy(mnemo[DIVGen], "DIV ");
    strcpy(mnemo[MODGen], "MOD ");
    strcpy(mnemo[CMPGen], "CMP ");
    strcpy(mnemo[MOVIGen], "MOVI ");
    strcpy(mnemo[MVNIGen], "MVNI ");
    strcpy(mnemo[ADDIGen], "ADDI ");
    strcpy(mnemo[SUBIGen], "SUBI ");
    strcpy(mnemo[MULIGen], "MULI ");
    strcpy(mnemo[DIVIGen], "DIVI ");
    strcpy(mnemo[MODIGen], "MODI ");
    strcpy(mnemo[CMPIGen], "CMPI ");
    strcpy(mnemo[CHKIGen], "CHKI ");
    strcpy(mnemo[LDWGen], "LDW ");
    strcpy(mnemo[LDBGen], "LDB ");
    strcpy(mnemo[POPGen], "POP ");
    strcpy(mnemo[STWGen], "STW ");
    strcpy(mnemo[STBGen], "STB ");
    strcpy(mnemo[PSHGen], "PSH ");
    strcpy(mnemo[BEQGen], "BEQ ");
    strcpy(mnemo[BNEGen], "BNE ");
    strcpy(mnemo[BLTGen], "BLT ");
    strcpy(mnemo[BGEGen], "BGE ");
    strcpy(mnemo[BLEGen], "BLE ");
    strcpy(mnemo[BGTGen], "BGT ");
    strcpy(mnemo[BRGen], "BR ");
    strcpy(mnemo[BSRGen], "BSR ");
    strcpy(mnemo[RETGen], "RET ");
    strcpy(mnemo[RDGen], "READ ");
    strcpy(mnemo[WRDGen], "WRD ");
    strcpy(mnemo[WRHGen], "WRH ");
    strcpy(mnemo[WRLGen], "WRL "); //заполнение массива кодов команд
    fprintf(outputFile, "entry %d\r\n", storage->entryAddress * 4);
    for(int i = 0; i < storage->PC; i++) {
        int codeCommand = storage->code[i];
        int command = (codeCommand >> 26) & 0x3F; //получение первых 5 бит - команда
        fprintf(outputFile, "%d %s", 4 * i, mnemo[command]);
        if(command < MOVIGen) {
            //c - регистр
            fprintf(outputFile, "%d, %d, %d\r\n", (codeCommand >> 22) & 0x0F, (codeCommand >> 18) & 0x0F,
                    codeCommand & 0x0F); //a - след. 4 бита, b - след. 4 бита, c - след. 18 бит
        }
        else if(command < BEQGen) {
            //c - константа либо смещение
            int c = codeCommand & 0x3FFFF;
            if(c >= 0x20000)
                c -= 0x40000; //если смещение
            fprintf(outputFile, "%d, %d, %d\r\n", (codeCommand >> 22) & 0x0F, (codeCommand >> 18) & 0x0F, c);
        }
        else {
            int c = codeCommand & 0x3FFFFFF;
            if(command == RETGen)
                fprintf(outputFile, "%d\r\n", c); //c - регистр
            else {
                //c - смещение
                if(c >= 0x2000000)
                    c -= 0x4000000;
                fprintf(outputFile, "%d\r\n", c * 4);
            }
        }
    }
    fprintf(outputFile, "%d bytes\r\n", storage->PC * 4);
    fclose(outputFile);
    printf("Compiled code is in the \"output.txt\" file in the current directory. Bye!\n");

}

void parameterGen(struct item *x, struct type *ftyp, struct parameters *storage) {

    //запись фактических передаваемых параметров функции в стек
    if(x->classType == ftyp) { //если параметр верный
        if(x->mode != RegGen)
            load(x, storage); //загрузка в регистр
        put(PSHGen, x->storage, SPGen, 4, storage); //загрузка в стек
        storage->registers[x->storage] = 0; //обнуление регистра
    }
    else
        mark("Bad parameter type", storage);

}
