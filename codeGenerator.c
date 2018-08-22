#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "scanner.h"
#include "codeGenerator.h"

int ASH(int a, int b) {

    return a * pow(2, b);

}

void TestRange(int x, struct parameters* storage) {

    if((x >= 0x20000) || (x < -0x20000))
        mark("value is too large", storage);

} //F1, F2: test im. a - 4, b - 4, im - 18

void Put(int op, int a, int b, int c, struct parameters* storage) {

    //запись сгенерированной команды
    int pc = storage->PC; //получение счетчика команд
    if(pc < maxCodeSize) { //если номер команды не привышает максимально допустимой длины кода
        storage->code[pc] = (struct machineCommand*)malloc(sizeof(struct machineCommand));
        storage->code[pc]->command = op;
        storage->code[pc]->a = a;
        storage->code[pc]->b = b;
        storage->code[pc]->c = c; //выделение памяти и запись параметров
        storage->PC++; //увеличение счетчика команд
    }
    else
        mark("Too large code", storage);

}

int GetReg(struct parameters* storage) {

    int i = 0;
    while((i < FPGen) && (storage->registers[i] == 1))
        i++;
    storage->registers[i] = 1;
    return i;

}

void load(struct Item* x, struct parameters* storage) {

    int r;
    if(x->mode == VarGen) {
        if(x->level == 0)
            x->a -= storage->PC * 4;
        r = GetReg(storage);
        Put(LDWGen, r, x->storage, x->a, storage);
        storage->registers[x->storage] = 0;
        x->storage = r;
    }
    else if(x->mode == ConstGen) {
        TestRange(x->a, storage);
        x->storage = GetReg(storage);
        Put(MOVIGen, x->storage, 0, x->a, storage);
    }
    x->mode = RegGen;

}

void Index(struct Item* x, struct Item* y, struct parameters* storage) {

    if(y->type != storage->intType)
        mark("index is not integer", storage);
    //x[y]
    if(y->mode == ConstGen) {
        if((y->a < 0) || (y->a >= x->type->length))
            mark("bad index", storage);
        x->a += y->a * x->type->baseType->size;
    }
    else {
        if(y->mode != RegGen) {}
            load(y, storage);
        Put(CHKIGen, y->storage, 0, x->type->length, storage);
        Put(MULIGen, y->storage, y->storage, x->type->baseType->size, storage);
        Put(ADDGen, y->storage, x->storage, y->storage, storage);
        storage->registers[x->storage] = 0;
        x->storage = y->storage;
    }
    x->type = x->type->baseType;

}

struct Object* FindField(struct Object* list, struct parameters* storage) {

    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));
    strcpy(storage->guard->name, storage->lastLexeme);
    while(strcmp(list->name, storage->lastLexeme))
        list = list->nextObject;
    obj = list;
    return obj;

}

void Field(struct Item* x, struct Object* y, struct parameters* storage) {

    x->a += y->value;
    x->type = y->type;

}

struct Item* MakeItem(struct Object* y, struct parameters* storage) {

    int r = 0;
    struct Item* x = (struct Item*)malloc(sizeof(struct Item));
    x->mode = y->class;
    x->type = y->type;
    x->level = y->level;
    x->a = y->value;
    x->b = 0;
    if(y->level == 0)
        x->storage = storage->PC;
    else if(y->level == storage->currentLevel)
        x->storage = FPGen;
    else {
        mark("level!", storage);
        x->storage = 0;
    }
    if(y->class = ParGen) {
        r = GetReg(storage);
        Put(LDWGen, r, x->storage, x->a, storage);
        x->mode = VarGen;
        x->storage = r;
        x->a = 0;
    }
    return x;

}

struct Item* MakeConstItem(struct Type* typ, int val, struct parameters* storage) {

    struct Item* x = (struct Item*)malloc(sizeof(struct Item));
    x->mode = ConstGen;
    x->type = typ;
    x->a = val;
    return x;

}

int negated(int cond) {

    if(cond % 2)
        return cond - 1;
    else
        return cond + 1;

}

void loadBool(struct Item* x, struct parameters* storage) {

    //тот же load, только с проверкой
    if(x->type->type != BooleanGen)
        mark("boolean?", storage);
    load(x, storage);
    x->mode = CondGen;
    x->a = 0;
    x->b = 0;
    x->c = 1;

}

void PutBR(int op, int disp, struct parameters* storage) {

    int pc = storage->PC; //получение счетчика команд
    if(pc < maxCodeSize) { //если номер команды не привышает максимально допустимой длины кода
        storage->code[pc] = (struct machineCommand*)malloc(sizeof(struct machineCommand));
        storage->code[pc]->command = op;
        storage->code[pc]->c = disp; //выделение памяти и запись параметров
        storage->PC++; //увеличение счетчика команд
    }
    else
        mark("Too large code", storage);

}

void fix(int at, int with, struct parameters* storage) {

    //storage->code[at] = (storage->code[at] / 0x400000) * 0x400000 + (with % 0x400000);

}

void FixLink(int L, struct parameters* storage) {

    int L1;
    while(L != 0) {
        //L1 = storage->code[L] % 0x40000;
        fix(L, storage->PC - L, storage);
        L = L1;
    }

}

void Op1(int op, struct Item* x, struct parameters* storage) {

    int t;

    if(op == minusLexical) {
        if(x->type->type != IntegerGen)
            mark("bad type", storage);
        else if(x->mode == ConstGen)
            x->a -= 2 * (x->a);
        else {
            if(x->mode == VarGen)
                load(x, storage);
            Put(MVNGen, x->storage, 0, x->storage, storage);
        }
    }
    else if(op == notLexical) {
        if(x->mode != CondGen)
            loadBool(x, storage);
        x->c = negated(x->c);
        t = x->a;
        x->a = x->b;
        x->b = t;
    }
    else if(op == andLexical) {
        if(x->mode != CondGen) {}
            loadBool(x, storage);
        PutBR(BEQGen + negated(x->c), x->a, storage);
        storage->registers[x->storage] = 0;
        x->a = storage->PC - 1;
        FixLink(x->b, storage);
        x->b = 0;
    }
    else if(op == orLexical) {
        if(x->mode != CondGen) {}
            loadBool(x, storage);
        PutBR(BEQGen + x->c, x->b, storage);
        storage->registers[x->storage] = 0;
        x->b = storage->PC - 1;
        FixLink(x->a, storage);
        x->a = 0;
    }

}

void PutOp(int cd, struct Item* x, struct Item* y, struct parameters* storage) {

    if(x->mode != RegGen)
        load(x, storage);
    if(y->mode == ConstGen) {
        TestRange(y->a, storage);
        Put(cd + 16, x->storage, x->storage, y->a, storage);
    }
    else {
        if(y->mode != RegGen)
            load(y, storage);
        Put(cd, x->storage, x->storage, y->storage, storage);
        storage->registers[y->storage] = 0;
    }

}

int merged(int L0, int L1, struct parameters* storage) {

    int L2, L3;
    if(L0 != 0) {
        L2 = L0;
        while(L3 != 0) {
            //L3 = storage->code[L2] % 0x40000;
            if(L3 != 0)
                L2 = L3;
        }
        storage->code[L2] = storage->code[L2] - L3 + L1;
        return L0;
    }
    else
        return L1;

}

void Op2(int op, struct Item* x, struct Item* y, struct parameters* storage) {

    //x = x op y;
    if((x->type->type == IntegerGen) && (y->type->type == IntegerGen)) {
        if((x->mode == ConstGen) && (y->mode == ConstGen)) {
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
                mark("bad type", storage);
        }
        else {
            if(op == plusLexical)
                PutOp(ADDGen, x, y, storage);
            else if(op == minusLexical)
                PutOp(SUBGen, x, y, storage);
            else if(op == timesLexical)
                PutOp(MULGen, x, y, storage);
            else if(op == divLexical)
                PutOp(DIVGen, x, y, storage);
            else if(op == modLexical)
                PutOp(MODGen, x, y, storage);
            else
                mark("bad type", storage);
        }
    }
    else if((x->type->type == BooleanGen) && (y->type->type == BooleanGen)) {
        if(y->mode != CondGen)
            loadBool(y, storage);
        if(op == orLexical) {
            x->a = y->a;
            x->b = merged(y->b, x->b, storage);
            x->c = y->c;
        }
        else if(op == andLexical) {
            x->a = merged(y->a, x->a, storage);
            x->b = y->b;
            x->c = y->c;
        }
    }
    else
        mark("bad type", storage);

}

void Relation(int op, struct Item* x, struct Item* y, struct parameters* storage) {

    if((x->type->type != IntegerGen) || (y->type->type != IntegerGen))
        mark("bad type", storage);
    else {
        PutOp(CMPGen, x, y, storage);
        x->c = op - eqlLexical;
        storage->registers[y->storage] = 0;
    }
    x->mode = CondGen;
    x->type = storage->boolType;
    x->a = 0;
    x->b = 0;

}

void IncLevel(int n, struct parameters* storage) {

    storage->currentLevel += n;

}

void Enter(int size, struct parameters* storage) {

    Put(PSHGen, LNKGen, SPGen, 4, storage);
    Put(PSHGen, FPGen, SPGen, 4, storage);
    Put(MOVGen, FPGen, 0, SPGen, storage);
    Put(SUBIGen, SPGen, SPGen, size, storage);

}

void Return(int size, struct parameters* storage) {

    Put(MOVGen, SPGen, 0, FPGen, storage);
    Put(POPGen, FPGen, SPGen, 4, storage);
    Put(POPGen, LNKGen, SPGen, size + 4, storage);
    PutBR(RETGen, LNKGen, storage);

}

void Store(struct Item* x, struct Item* y, struct parameters* storage) { //x = y

    int r;
    if((x->type->type == BooleanGen || x->type->type == IntegerGen) && (x->type->type == y->type->type)) {
        if(y->mode == CondGen) {
            Put(BEQGen + negated(y->c), y->storage, 0, y->a, storage);
            storage->registers[y->storage] = 0;
            y->a = storage->PC - 1;
            FixLink(y->b, storage);
            y->storage = GetReg(storage);
            Put(MOVIGen, y->storage, 0, 1, storage);
            PutBR(BRGen, 2, storage);
            FixLink(y->a, storage);
            Put(MOVIGen, y->storage, 0, 0, storage);
        }
        else if(y->mode != RegGen)
            load(y, storage);
        if(x->mode == VarGen) {
            if(x->level == 0)
                x->a = x->a - (storage->PC) * 4;
            Put(STWGen, y->storage, x->storage, x->a, storage);
        }
        else
            mark("illegal assignment", storage);
        storage->registers[x->storage] = 0;
        storage->registers[y->storage] = 0;
    }
    else
        mark("incompatible assignment", storage);

}

void Call(struct Item* x, struct parameters* storage) {

    PutBR(BSRGen, x->a - storage->PC, storage);

}

void IOCall(struct Item* x, struct Item* y, struct parameters* storage) {

    struct Item* z = (struct Item*)malloc(sizeof(struct Item));
    if(x->a < 4) {
        if(y->type->type != IntegerGen)
            mark("Integer?", storage);
    }
    if(x->a == 1) { //read
        z->storage = GetReg(storage);
        z->mode = RegGen;
        z->type = storage->intType;
        Put(RDGen, z->storage, 0, 0, storage);
        Store(y, z, storage);
    }
    else if(x->a == 2) { //write
        load(y, storage);
        Put(WRDGen, 0, 0, y->storage, storage);
        storage->registers[y->storage] = 0;
    }
    else if(x->a == 3) { //writehex
        load(y, storage);
        Put(WRHGen, 0, 0, y->storage, storage);
        storage->registers[y->storage] = 0;
    }
    else //writeln
        Put(WRLGen, 0, 0, 0, storage);

}

void CJump(struct Item* x, struct parameters* storage) {

    if(x->type->type == BooleanGen) {
        if(x->mode != CondGen)
            loadBool(x, storage);
        PutBR(BEQGen + negated(x->c), x->a, storage);
        storage->registers[x->storage] = 0;
        FixLink(x->b, storage);
        x->a = storage->PC - 1;
    }
    else {
        mark("Boolean?", storage);
        x->a = storage->PC;
    }

}

void BJump(int L, struct parameters* storage) {

    PutBR(BRGen, L - storage->PC, storage);

}

int FJump(int L, struct parameters* storage) {

    int retL;
    PutBR(BRGen, L, storage);
    retL = storage->PC - 1;
    return retL;

}

void Header(int size, struct parameters* storage) {

    storage->entryAddress = storage->PC;
    Put(MOVIGen, SPGen, 0, 1024 - size, storage); //todo RISC.Memsize
    Put(PSHGen, LNKGen, SPGen, 4, storage);

}

void Close(struct parameters* storage) {

    Put(POPGen, LNKGen, SPGen, 4, storage);
    PutBR(RETGen, LNKGen, storage);

}

void decode(struct parameters* storage) {

    //запись кода в файл
    int i;
    FILE* outputFile = NULL;
    printf("Recording compiled code...\n");
    if((outputFile = fopen("output.txt", "wb")) == NULL) {
        printf("Opening output file error!\n");
        fprintf(storage->reportFile, "Opening output file error!\r\n");
        return;
    }
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
    strcpy(mnemo[WRLGen], "WRL ");
    fprintf(outputFile, "entry %d\r\n", storage->entryAddress * 4);
    for(i = 0; i < storage->PC; i++) {
        fprintf(outputFile, "%d %s", 4 * i, mnemo[storage->code[i]->command]);
        if(storage->code[i]->command < BEQGen)
            fprintf(outputFile, "%d, %d, ", storage->code[i]->a, storage->code[i]->b);
        fprintf(outputFile, "%d\r\n", storage->code[i]->c);
    }
    fclose(outputFile);
    printf("Compiled code is in the \"output.txt\" file in the current directory. Bye!\n");

}

void Parameter(struct Item* x, struct Type* ftyp, int class, struct parameters* storage) {

    int r;
    if(x->type == ftyp) {
        if(class == ParGen) {
            if(x->mode == VarGen) {
                if(x->a != 0) {
                    r = GetReg(storage);
                    Put(ADDIGen, r, x->storage, x->a, storage);
                }
                else
                    r = x->storage;
            }
            else
                mark("Illegal parameter mode", storage);
            Put(PSHGen, r, SPGen, 4, storage);
            storage->registers[r] = 0;
        }
        else {
            if(x->mode != RegGen)
                load(x, storage);
            Put(PSHGen, x->storage, SPGen, 4, storage);
            storage->registers[x->storage] = 0;
        }
    }
    else
        mark("Bad parameter type", storage);

}