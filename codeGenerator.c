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