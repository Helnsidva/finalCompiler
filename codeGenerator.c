#include <malloc.h>
#include <math.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"
#include "codeGenerator.h"

int ASH(int a, int b) {

    return a * pow(2, b);

}
//todo test it

void TestRange(int x, struct parameters* storage) {

    if((x >= 0x20000) || (x < -0x20000))
        Mark("value is too large", storage);

} //F1, F2: test im. a - 4, b - 4, im - 18

void Put(int op, int a, int b, int c, struct parameters* storage) {

    if(op >= 32)
        op -= 64;

    storage->code[storage->pc] = ASH(ASH(ASH(op, 4) + a, 4) + b, 18) + (c % 0x40000);
    storage->pc++;

}
//todo test it

int GetReg(struct parameters* storage) {

    int i = 0;
    while((i < FPGen) && (storage->regs[i] == 1))
        i++;
    storage->regs[i] = 1;
    return i;

}
//todo test it

void load(struct Item* x, struct parameters* storage) {

    int r;
    if(x->mode == VarGen) {
        if(x->lev == 0)
            x->a -= storage->pc * 4;
        r = GetReg(storage);
        Put(LDWGen, r, x->r, x->a, storage);
        storage->regs[x->r] = 0;
        x->r = r;
    }
    else if(x->mode == ConstGen) {
        TestRange(x->a, storage);
        x->r = GetReg(storage);
        Put(MOVIGen, x->r, 0, x->a, storage);
    }
    x->mode = RegGen;

}
//todo test it

void Index(struct Item* x, struct Item* y, struct parameters* storage) {

    if(y->type != storage->intType)
        Mark("index is not integer", storage);
    //x[y]
    if(y->mode == ConstGen) {
        if((y->a < 0) || (y->a >= x->type->len))
            Mark("bad index", storage);
        x->a += y->a * x->type->base->size;
    }
    else {
        if(y->mode != RegGen) {}
            load(y, storage);
        Put(CHKIGen, y->r, 0, x->type->len, storage);
        Put(MULIGen, y->r, y->r, x->type->base->size, storage);
        Put(ADDGen, y->r, x->r, y->r, storage);
        storage->regs[x->r] = 0;
        x->r = y->r;
    }
    x->type = x->type->base;

}
//todo test it

struct Object* FindField(struct Object* list, struct parameters* storage) {

    struct Object* obj = (struct Object*)malloc(sizeof(struct Object));
    strcpy(storage->guard->name, storage->lastLexeme);
    while(strcmp(list->name, storage->lastLexeme))
        list = list->next;
    obj = list;
    return obj;

}
//todo test it

void Field(struct Item* x, struct Object* y, struct parameters* storage) {

    x->a += y->val;
    x->type = y->type;

}
//todo test it

struct Item* MakeItem(struct Object* y, struct parameters* storage) {

    int r = 0;
    struct Item* x = (struct Item*)malloc(sizeof(struct Item));
    x->mode = y->class;
    x->type = y->type;
    x->lev = y->lev;
    x->a = y->val;
    x->b = 0;
    if(y->lev == 0)
        x->r = storage->pc;
    else if(y->lev == storage->curlev)
        x->r = FPGen;
    else {
        Mark("level!", storage);
        x->r = 0;
    }
    if(y->class = ParGen) {
        r = GetReg(storage);
        Put(LDWGen, r, x->r, x->a, storage);
        x->mode = VarGen;
        x->r = r;
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
    if(x->type->form != BooleanGen)
        Mark("boolean?", storage);
    load(x, storage);
    x->mode = CondGen;
    x->a = 0;
    x->b = 0;
    x->c = 1;

}

void PutBR(int op, int disp, struct parameters* storage) {

    storage->code[storage->pc] = ASH(op - 0x40, 26) + (disp % 0x4000000);
    storage->pc++;

}
//todo

void fix(int at, int with, struct parameters* storage) {

    storage->code[at] = (storage->code[at] / 0x400000) * 0x400000 + (with % 0x400000);

}
//todo

void FixLink(int L, struct parameters* storage) {

    int L1;
    while(L != 0) {
        L1 = storage->code[L] % 0x40000;
        fix(L, storage->pc - L, storage);
        L = L1;
    }

}
//todo

void Op1(struct Item* x, int op, struct parameters* storage) {

    int t;

    if(op == minusLexical) {
        if(x->type->form != IntegerGen)
            Mark("bad type", storage);
        else if(x->mode == ConstGen)
            x->a -= 2 * (x->a);
        else {
            if(x->mode == VarGen)
                load(x, storage);
            Put(MVNGen, x->r, 0, x->r, storage);
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
        storage->regs[x->r] = 0;
        x->a = storage->pc - 1;
        FixLink(x->b, storage);
        x->b = 0;
    }
    else if(op == orLexical) {
        if(x->mode != CondGen) {}
            loadBool(x, storage);
        PutBR(BEQGen + x->c, x->b, storage);
        storage->regs[x->r] = 0;
        x->b = storage->pc - 1;
        FixLink(x->a, storage); //todo
        x->a = 0;
    }

}
//todo

void PutOp(int cd, struct Item* x, struct Item* y, struct parameters* storage) {

    if(x->mode != RegGen)
        load(x, storage);
    if(y->mode == ConstGen) {
        TestRange(y->a, storage);
        Put(cd + 16, x->r, x->r, y->a, storage);
    }
    else {
        if(y->mode != RegGen)
            load(y, storage);
        Put(cd, x->r, x->r, y->r, storage);
        storage->regs[y->r] = 0;
    }

}

int merged(int L0, int L1, struct parameters* storage) {

    int L2, L3;
    if(L0 != 0) {
        L2 = L0;
        while(L3 != 0) {
            L3 = storage->code[L2] % 0x40000;
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
    if((x->type->form == IntegerGen) && (y->type->form == IntegerGen)) {
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
                Mark("bad type", storage);
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
                Mark("bad type", storage);
        }
    }
    else if((x->type->form == BooleanGen) && (y->type->form == BooleanGen)) {
        if(y->mode != CondGen)
            loadBool(y, storage);
        if(op == orLexical) {
            x->a = y->a;
            x->b = merged(y->b, x->b, storage); //todo
            x->c = y->c;
        }
        else if(op == andLexical) {
            x->a = merged(y->a, x->a, storage); //todo
            x->b = y->b;
            x->c = y->c;
        }
    }
    else
        Mark("bad type", storage);

}

void Relation(int op, struct Item* x, struct Item* y, struct parameters* storage) {

    if((x->type->form != IntegerGen) || (y->type->form != IntegerGen))
        Mark("bad type", storage);
    else {
        PutOp(CMPGen, x, y, storage);
        x->c = op - eqlLexical;
        storage->regs[y->r] = 0;
    }
    x->mode = CondGen;
    x->type = storage->boolType;
    x->a = 0;
    x->b = 0;

}

void IncLevel(int n, struct parameters* storage) {

    storage->curlev += n;

}

void EnterCmd(char name[], struct parameters* storage) {

    strcpy(storage->comname[storage->cno], name);
    storage->comadr[storage->cno] = storage->pc * 4;
    storage->cno++;

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