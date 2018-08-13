//class / mode numb identifiers
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

//types numb identifiers
#define BooleanGen 0
#define IntegerGen 1
#define ArrayGen 2
#define RecordGen 3

//assembly commands numb identifiers
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

//reserved registers numb identifiers
#define FPGen 12
#define SPGen 13
#define LNKGen 14
#define PCGen 15

void Index(struct Item*, struct Item*, struct parameters*);
struct Object* FindField(struct Object*, struct parameters*);
void Field(struct Item*, struct Object*, struct parameters*);
struct Item* MakeItem(struct Object*, struct parameters*);
struct Item* MakeConstItem(struct Type*, int, struct parameters*);
void Op1(struct Item*, int, struct parameters*);