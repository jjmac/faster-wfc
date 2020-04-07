#define FIELD_LEN 64

typedef unsigned long long field;
typedef struct bitmask * Bitmask;

struct bitmask {
    int len;
    field * fields;
};

Bitmask bmCreate(int len);
void bmDestroy(Bitmask old);
void bmPrint(Bitmask self);
void bmFieldPrint(field cur);

int bmFalse(Bitmask self);
int bmTrue(Bitmask self);

void bmClear(Bitmask self);
void bmNot(Bitmask self);
void bmCopy(Bitmask self, Bitmask other);
void bmAnd(Bitmask self, Bitmask other);
void bmOr(Bitmask self, Bitmask other);
void bmXor(Bitmask self, Bitmask other);

int bmAndValue(Bitmask self, Bitmask other);
int bmXorValue(Bitmask self, Bitmask other);

void bmCherrypick(Bitmask self, unsigned int * output);
void bmFastCherrypick(Bitmask self, unsigned int * output);

Bitmask nbmCopy(Bitmask self);
Bitmask nbmNot(Bitmask self);
Bitmask nbmAnd(Bitmask b1, Bitmask b2);
Bitmask nbmOr(Bitmask b1, Bitmask b2);
Bitmask nbmXor(Bitmask b1, Bitmask b2);
