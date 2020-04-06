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

void bmFieldPrint(field cur) {
    for (field digitMask = HIGH_BIT; digitMask>0; digitMask = digitMask >> 1) {
        putchar(cur&digitMask ? '1' : '0');
    }
}

void bmPrint(Bitmask self) {
    field cur;
    for(int k = self->len-1; k >= 0; k--) {
        cur = self->fields[k];

        #if DPRINT_BINARY
            for (field digitMask = HIGH_BIT; digitMask>0; digitMask = digitMask >> 1) {
                putchar(cur&digitMask ? '1' : '0');
            }
        # else
            printf("%llu ", cur);
        #endif
        putchar('_');
    }
}

int bmFalse(Bitmask self) {
    for(int k = self->len-1; k >= 0; k--) {
        if (self->fields[k]) {
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


void bmCopy(Bitmask self, Bitmask other) {
    assert(self->len == other->len);
    for(int k = self->len - 1; k >= 0; k--) {
        self->fields[k] = other->fields[k];
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

int bmAndValue(Bitmask self, Bitmask other) {
    assert(self->len == other->len);
    for(int k = self->len - 1; k >= 0; k--) {
        if (self->fields[k] & other->fields[k]) {
            return 1;
        }
    }
    return 0;
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

unsigned int VLOOKUPS[67] = { 0, 0, 1, 39, 2, 15, 40, 23, 3, 12, 16, 59, 41, 19, 24, 54, 4, 0, 13, 10, 17,
62, 60, 28, 42, 30, 20, 51, 25, 44, 55, 47, 5, 32, 0, 38, 14, 22, 11, 58,
18, 53, 63, 9, 61, 27, 29, 50, 43, 46, 31, 37, 21, 57, 52, 8, 26, 49, 45, 36,
56, 7, 48, 35, 6, 34, 33 };  // A mapping from (2^n) % 67 to n.  Has the property that 2^n % 67 gives a unique result for all n in 0..64

/// does not work!
void bmCherrypick(Bitmask self, unsigned int * output) {
    int oLen = 1;
    for(int k = self->len-1; k >= 0; k--) {
        while (self->fields[k]) {
            field nextVal = ~(self->fields[k]-1);
            self->fields[k] &= nextVal;
            output[oLen++] = nextVal & self->fields[k];
        }
    }
    output[0] = oLen;
}

void bmFastCherrypick(Bitmask self, unsigned int * output) {
//    printf("    Running a fast cherrypick of bitmask:");
//    bmPrint(self);
//    printf("\n");
//    printf("    Output x is %p\n", output);
    int oLen = 0;
    for(unsigned int k = self->len-1; k != -1; k--) {
//        printf("    -- Checking field %d it is %llx\n", k, self->fields[k]);
        while (self->fields[k]) {
            field nextVal = (self->fields[k]-1) & self->fields[k];
//            printf("    %llu & %llu = Nextval is %llu\n", self->fields[k]-1ULL, self->fields[k], nextVal);
            field intermNextVal = (nextVal ^ self->fields[k]);
//            printf("    Intermediate nextval is %llu\n", intermNextVal);
//            printf("    Prevlookups is %llu\n", intermNextVal % 67ULL);
            intermNextVal = VLOOKUPS[intermNextVal % 67ULL];
//            printf("    Intermediate-2 nextval is %llu\n", intermNextVal);
//            printf("    Reduced nextVal is %u\n", VLOOKUPS[(nextVal ^ self->fields[k]) % 67] + (FIELD_LEN * k));
            output[++oLen] = VLOOKUPS[(nextVal ^ self->fields[k]) % 67] + (FIELD_LEN * k);
            self->fields[k] = nextVal;
//            printf("    Aftand selfield is %llx\n", self->fields[k]);
        }
    }
//    printf("    doing output (oLen %d)\n", oLen);
    output[0] = oLen;
}
