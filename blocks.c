#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"

#define DPRINT_OVERLAPS  0
#define DPRINT_OVERLAP_CHECKS  0

#define flat2(x,y) (x+y*2)
#define flat3(x,y) (x+y*3)
#define flat4(x,y) (x+y*4)
#define flat(x,y, size) (x+y*size)

#define unflatX(xy, size) (xy%size)
#define unflatY(xy, size) (xy/size)

#define OVERLAPS_LEN(size) (size * (size-1))

const unsigned char nOverlaps4[24] = {
    flat4(0,0), flat4(0,1),
    flat4(1,0), flat4(1,1),
    flat4(2,0), flat4(2,1),
    flat4(3,0), flat4(3,1),

    flat4(0,1), flat4(0,2),
    flat4(1,1), flat4(1,2),
    flat4(2,1), flat4(2,2),
    flat4(3,1), flat4(3,2),

    flat4(0,2), flat4(0,3),
    flat4(1,2), flat4(1,3),
    flat4(2,2), flat4(2,3),
    flat4(3,2), flat4(3,3),
};
const unsigned char sOverlaps4[24] = {
    flat4(0,1), flat4(0,0),
    flat4(1,1), flat4(1,0),
    flat4(2,1), flat4(2,0),
    flat4(3,1), flat4(3,0),

    flat4(0,2), flat4(0,1),
    flat4(1,2), flat4(1,1),
    flat4(2,2), flat4(2,1),
    flat4(3,2), flat4(3,1),

    flat4(0,3), flat4(0,2),
    flat4(1,3), flat4(1,2),
    flat4(2,3), flat4(2,2),
    flat4(3,3), flat4(3,2),
};
const unsigned char eOverlaps4[24] = {
    flat4(0,0), flat4(1,0),
    flat4(0,1), flat4(1,1),
    flat4(0,2), flat4(1,2),
    flat4(0,3), flat4(1,3),

    flat4(1,0), flat4(2,0),
    flat4(1,1), flat4(2,1),
    flat4(1,2), flat4(2,2),
    flat4(1,3), flat4(2,3),

    flat4(2,0), flat4(3,0),
    flat4(2,1), flat4(3,1),
    flat4(2,2), flat4(3,2),
    flat4(2,3), flat4(3,3),
};
const unsigned char wOverlaps4[24] = {
    flat4(1,0), flat4(0,0),
    flat4(1,1), flat4(0,1),
    flat4(1,2), flat4(0,2),
    flat4(1,3), flat4(0,3),

    flat4(2,0), flat4(1,0),
    flat4(2,1), flat4(1,1),
    flat4(2,2), flat4(1,2),
    flat4(2,3), flat4(1,3),

    flat4(3,0), flat4(2,0),
    flat4(3,1), flat4(2,1),
    flat4(3,2), flat4(2,2),
    flat4(3,3), flat4(2,3),
};


const unsigned char nOverlaps3[12] = {
    flat3(0,0), flat3(0,1),
    flat3(1,0), flat3(1,1),
    flat3(2,0), flat3(2,1),

    flat3(0,1), flat3(0,2),
    flat3(1,1), flat3(1,2),
    flat3(2,1), flat3(2,2),
};
const unsigned char sOverlaps3[12] = {
    flat3(0,1), flat3(0,0),
    flat3(1,1), flat3(1,0),
    flat3(2,1), flat3(2,0),

    flat3(0,2), flat3(0,1),
    flat3(1,2), flat3(1,1),
    flat3(2,2), flat3(2,1),
};
const unsigned char eOverlaps3[12] = {
    flat3(0,0), flat3(1,0),
    flat3(0,1), flat3(1,1),
    flat3(0,2), flat3(1,2),

    flat3(1,0), flat3(2,0),
    flat3(1,1), flat3(2,1),
    flat3(1,2), flat3(2,2)
};
const unsigned char wOverlaps3[12] = {
    flat3(1,0), flat3(0,0),
    flat3(1,1), flat3(0,1),
    flat3(1,2), flat3(0,2),

    flat3(2,0), flat3(1,0),
    flat3(2,1), flat3(1,1),
    flat3(2,2), flat3(1,2)
};


const unsigned char nOverlaps2[4] = {
    flat2(0,0), flat2(0,1),
    flat2(1,0), flat2(1,1)
};
const unsigned char sOverlaps2[4] = {
    flat2(0,1), flat2(0,0),
    flat2(1,1), flat2(1,0)
};
const unsigned char wOverlaps2[4] = {
    flat2(0,0), flat2(1,0),
    flat2(0,1), flat2(1,1)
};
const unsigned char eOverlaps2[4] = {
    flat2(1,0), flat2(0,0),
    flat2(1,1), flat2(0,1)
};

static void blPrintOverlaps(Block self, cardinal dir);

Block blCreate(int size){
    Block self = malloc(sizeof(struct block));
    self->freq = 1;
    self->size = size;
    self->values = malloc(sizeof(char) * self->size * self->size);
    return self;
}
void blDestroy(Block self){
    free(self->values);
    free(self);
}

Block blCreateFromString(int size, char* str) {
    Block self = blCreate(size);
    for (int k = self->size*self->size - 1; k >= 0; k--) {
        self->values[k] = str[k];
    }
    return self;
}

void blPrint(Block self){
    int x = 0;
    for (int y = 0; y < self->size; y++) {
        for (x = 0; x < self->size; x++) {
            putchar(self->values[flat(x, y, self->size)]);
        }
        putchar('\n');
    }
}
void blPrintData(Block self){
    printf("  absIndex# %d (fieldIndex %d / bitIndex %d)\n", absIndex(self), self->fieldIndex, self->bitIndex);
    blPrintOverlaps(self, CARD_N);
    blPrintOverlaps(self, CARD_S);
    blPrintOverlaps(self, CARD_E);
    blPrintOverlaps(self, CARD_W);
}

static void blPrintOverlaps(Block self, cardinal dir){
    printf("  overlapmask-%c:", cardChar(dir));
    bmPrint(self->overlapMasks[dir]);
    printf("\n");
}

// TODO: replace with macro??
void blbmAdd(Block self, Bitmask bm) {
    bm->fields[self->fieldIndex] |= self->localMask;
}
void blbmRemove(Block self, Bitmask bm) {
    bm->fields[self->fieldIndex] ^= self->localMask;
}
// TODO: improve efficiency??
void blbmSet(Block self, Bitmask bm) {
    bmClear(bm);
    bm->fields[self->fieldIndex] |= self->localMask;
}
int blbmContains(Block self, Bitmask bm) {
/*    printf ("        Running blbmContains between:\n        --");
    bmFieldPrint(bm->fields[self->fieldIndex]);
    printf("\n        --");
    bmFieldPrint(self->localMask);
    printf("\n");*/
    return (bm->fields[self->fieldIndex] & self->localMask) > 0;
}


/*
int blCouldOverlap2(Block self, Block other, cardinal dir) {
    switch(self->size) {
        case 2:
            break;
        case 3:
            assert(1==0);
        case 4:
            assert(1==0);
        default:
            assert(1==0);
    }
}
*/

void blMarkOverlaps(Block self, Block other, cardinal dir) {
    #if DPRINT_OVERLAP_CHECKS
        printf("marking overlaps %d->%d (direction %c)!\n", absIndex(self), absIndex(other), cardChar(dir));
    #endif

    #if DPRINT_OVERLAPS
        printf("BLMarking Overlaps (direction %c)!\n", cardChar(dir));
        blPrint(self);
        printf("-- v --\n");
        blPrint(other);
    #endif

    const unsigned char * oPairs = NULL;
    switch(dir){
        case CARD_N:
            switch(self->size) {
                case 2:
                    oPairs = nOverlaps2;
                    break;
                case 3:
                    oPairs = nOverlaps3;
                    break;
                case 4:
                    oPairs = nOverlaps4;
                    break;
                default:
                    assert(1==0);
            }
            break;
        case CARD_W:
            switch(self->size) {
                case 2:
                    oPairs = wOverlaps2;
                    break;
                case 3:
                    oPairs = wOverlaps3;
                    break;
                case 4:
                    oPairs = wOverlaps4;
                    break;
                default:
                    assert(1==0);
            }
            break;
        case CARD_E:
            switch(self->size) {
                case 2:
                    oPairs = eOverlaps2;
                    break;
                case 3:
                    oPairs = eOverlaps3;
                    break;
                case 4:
                    oPairs = eOverlaps4;
                    break;
                default:
                    assert(1==0);
            }
            break;
        case CARD_S:
            switch(self->size) {
                case 2:
                    oPairs = sOverlaps2;
                    break;
                case 3:
                    oPairs = sOverlaps3;
                    break;
                case 4:
                    oPairs = sOverlaps4;
                    break;
                default:
                    assert(1==0);
            }
            break;
    }

    for (int k=0; k<2*OVERLAPS_LEN(self->size); k+= 2) {
        #if DPRINT_OVERLAPS
            printf("Comparing pos %d (%c) to (%c) %d\n", oPairs[k], self->values[oPairs[k]], other->values[oPairs[k+1]], oPairs[k+1]);
        #endif
        if (self->values[ oPairs[k] ] != other->values[oPairs[k+1]] ) {
            #if DPRINT_OVERLAPS
                printf("No overlap - quitting\n\n");
            #endif
            return;
        }
    }
    #if DPRINT_OVERLAPS
        printf("Overlap found!\n\n");
    #endif
    blbmAdd(self, other->overlapMasks[dir]);
}
