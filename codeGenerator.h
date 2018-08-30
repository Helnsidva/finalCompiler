//классы, режимы
#define HeadGen 0
#define VarGen 1
#define ParGen 2
#define ConstGen 3
#define FldGen 4
#define TypGen 5
#define ProcGen 6
#define SProcGen 7
#define RegGen 10
#define CondGen 11

//идентификаторы типов
#define BooleanGen 0
#define IntegerGen 1
#define ArrayGen 2
#define RecordGen 3

//ассемблерные команды
#define MOVGen 0
#define MVNGen 1
#define ADDGen 2
#define SUBGen 3
#define MULGen 4
#define DIVGen 5
#define MODGen 6
#define CMPGen 7
#define MOVIGen 16
#define MVNIGen 17
#define ADDIGen 18
#define SUBIGen 19
#define MULIGen 20
#define DIVIGen 21
#define MODIGen 22
#define CMPIGen 23
#define CHKIGen 24
#define LDWGen 32
#define LDBGen 33
#define POPGen 34
#define STWGen 36
#define STBGen 37
#define PSHGen 38
#define RDGen 40
#define WRDGen 41
#define WRHGen 42
#define WRLGen 43
#define BEQGen 48
#define BNEGen 49
#define BLTGen 50
#define BGEGen 51
#define BLEGen 52
#define BGTGen 53
#define BRGen 56
#define BSRGen 57
#define RETGen 58

//зарезервированные регистры
#define FPGen 12
#define SPGen 13
#define LNKGen 14
#define PCGen 15

void arrayElem(struct item *, struct item *, struct parameters *);
struct object* findField(struct object *, struct parameters *);
void getField(struct item *, struct object *);
struct item* makeItem(struct object *, struct parameters *);
struct item* makeConstItem(struct type *, int);
void singleGenerate(int, struct item *, struct parameters *);
void termGenerate(int, struct item *, struct item *, struct parameters *);
void relation(int, struct item *, struct item *, struct parameters *);
void changeLevel(int, struct parameters *);
void prologue(int, struct parameters *);
void epilogue(int, struct parameters *);
void fixLink(int, struct parameters *);
void store(struct item *, struct item *, struct parameters *);
void procedureCall(struct item *, struct parameters *);
void globalCall(struct item *, struct item *, struct parameters *);
void falseJump(struct item *, struct parameters *);
void whileJump(int, struct parameters *);
int elseJump(int, struct parameters *);
void codeHeader(int, struct parameters *);
void codeEnding(struct parameters *);
void decode(struct parameters*);
void parameterGen(struct item *, struct type *, int, struct parameters *);