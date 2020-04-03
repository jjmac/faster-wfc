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

int VLOOKUPS[67] = { 0, 0, 1, 39, 2, 15, 40, 23, 3, 12, 16, 59, 41, 19, 24, 54, 4, 0, 13, 10, 17,
62, 60, 28, 42, 30, 20, 51, 25, 44, 55, 47, 5, 32, 0, 38, 14, 22, 11, 58,
18, 53, 63, 9, 61, 27, 29, 50, 43, 46, 31, 37, 21, 57, 52, 8, 26, 49, 45, 36,
56, 7, 48, 35, 6, 0, 0 };

unsigned int * bmCherrypick(Bitmask self) {
    unsigned int * output = malloc(sizeof(unsigned int) * self->len * FIELD_LEN);
    int oLen = 1;
    for(int k = self->len-1; self >= 0; self--) {
        if (self->fields[k]) {
            field retVal = ~(self->fields[k]-1);
            self->fields[k] &= retVal;
            output[oLen++] = retVal & self->fields[k];
        }
    }
    output[0] = oLen;
    return output;
}

unsigned int * bmFastCherrypick(Bitmask self){
    unsigned int * output = malloc(sizeof(unsigned int) * (self->len * FIELD_LEN + 1));
    int oLen = 1;
    for(int k = self->len-1; self >= 0; self--) {
        if (self->fields[k]) {
            field retVal = ~(self->fields[k]-1);
            self->fields[k] &= retVal;
            output[oLen++] = VLOOKUPS[retVal % 67] + (FIELD_LEN * k);
        }
    }
    output[0] = oLen;
    return output;
}
