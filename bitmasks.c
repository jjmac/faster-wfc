#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "bitmasks.h"

#define DPRINT_BINARY 1

#define HIGH_BIT (1ULL << (FIELD_LEN-1))

Bitmask bmCreate(int len) {
    Bitmask self = malloc(sizeof(struct bitmask));
    self->len = len;
    self->fields = malloc(len * sizeof(field));
    return self;
}
void bmDestroy(Bitmask self) {
    free(self->fields);
    free(self);
}

void bmPrint(Bitmask self) {
    field cur;
    for(int k = 0; k < self->len; k++) {
        cur = self->fields[k];

        #if DPRINT_BINARY
            for (field digitMask = HIGH_BIT; digitMask>0; digitMask = digitMask >> 1) {
                putchar(cur&digitMask ? '1' : '0');
            }
        # else
            printf("%llu ", cur);
        #endif
    }
}

int bmFalse(Bitmask self) {
    for(int k = self->len-1; k >= 0; k--) {
        if (!(self->fields[k])) {
            return 0;
        }
    }
    return 1;
}
int bmTrue(Bitmask self) {
    for(int k = self->len-1; k >= 0; k--) {
        if (self->fields[k]) {
            return 1;
        }
    }
    return 0;
}

void bmClear(Bitmask self) {
    for(int k = self->len - 1; k >= 0; k--) {
        self->fields[k] = 0ULL;
    }
}
void bmNot(Bitmask self) {
    for(int k = self->len - 1; k >= 0; k--) {
        self->fields[k] = ~self->fields[k];
    }
}


void bmAnd(Bitmask self, Bitmask other) {
    assert(self->len == other->len);
    for(int k = self->len - 1; k >= 0; k--) {
        self->fields[k] &= other->fields[k];
    }
}
void bmOr(Bitmask self, Bitmask other) {
    assert(self->len == other->len);
    for(int k = self->len - 1; k >= 0; k--) {
        self->fields[k] |= other->fields[k];
    }
}
void bmXor(Bitmask self, Bitmask other) {
    assert(self->len == other->len);
    for(int k = self->len - 1; k >= 0; k--) {
        self->fields[k] ^= other->fields[k];
    }
}

Bitmask nbmCopy(Bitmask self){
    Bitmask new = bmCreate(self->len);
    for(int k = self->len-1; k >= 0; k--) {
        new->fields[k] = self->fields[k];
    }
    return new;
}
Bitmask nbmAnd(Bitmask b1, Bitmask b2){
    assert(b1->len == b2->len);
    Bitmask new = bmCreate(b1->len);
    for(int k = b1->len-1; k >= 0; k--) {
        new->fields[k] = b1->fields[k] & b2->fields[k];
    }
    return new;
}

Bitmask nbmOr(Bitmask b1, Bitmask b2){
    assert(b1->len == b2->len);
    Bitmask new = bmCreate(b1->len);
    for(int k = b1->len-1; k >= 0; k--) {
        new->fields[k] = b1->fields[k] | b2->fields[k];
    }
    return new;
}

Bitmask nbmXor(Bitmask b1, Bitmask b2){
    assert(b1->len == b2->len);
    Bitmask new = bmCreate(b1->len);
    for(int k = b1->len-1; k >= 0; k--) {
        new->fields[k] = b1->fields[k] ^ b2->fields[k];
    }
    return new;
}

unsigned int * bmCherrypick(Bitmask self) {
    unsigned int * output = malloc(sizeof(unsigned int) * self->len * FIELD_LEN);
    for(int k = self->len-1; self >= 0; self--) {
        if (self->fields[k]) {
            field retVal = ~(self->fields[k]-1);
            self->fields[k] &= retVal;
            return k * FIELD_LEN;

            return retVal & self->fields[k];
        }
    }
    return 0;
}
/*
int main(int argc, char** argv){
    Bitmask bm1 = bmCreate(3);
    Bitmask bm2 = bmCreate(3);

    bm1->fields[0] = 11;
    bm1->fields[2] = 13;

    bmOr(bm1, bm2);
    bmPrint(bm1);
    printf("\n");
}
*/
