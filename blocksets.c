#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "cardinals.h"
#include "grids.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"

#include "benchmarking.h"

#define DPRINT_LOCKED_DATA 1

float bEntropyTime = 0;

typedef struct blockListNode * BlockListNode;
struct blockListNode {
    BlockListNode next;
    Block value;
};

struct blockSet {
    unsigned char locked;
    unsigned char size;
    unsigned int len;

    Bitmask allTrueMask;
    Block * blocks;
    BlockListNode blockList;
};

static void bsetPrintLocked(BlockSet self);
static void bsetPrintUnlocked(BlockSet self);
static void bsetDestroyLocked(BlockSet self);
static void bsetDestroyUnlocked(BlockSet self);

// TODO: create BlockSet2, BlockSet3, BlockSet4, etc...
BlockSet bsetCreate(unsigned char size) {
    BlockSet self = malloc(sizeof(struct blockSet));
    self->size = size;
    self->locked = 0;
    self->len = 0;
    self->blockList = NULL;
    return self;
}

enum transform {
    IN_PLACE,
    ROT_90,
    ROT_180,
    ROT_270,
    REFL,
    REFL_ROT_90,
    REFL_ROT_180,
    REFL_ROT_270,
    INVALID_TRANSFORM
};

BlockSet bsetCreateFromGrid(Grid grid, unsigned char size, int rotations, int reflections) {
    BlockSet self = bsetCreate(size);

    unsigned int xSize = grid->xSize;
    unsigned int ySize = grid->ySize;

    char * allStrs[xSize * ySize * 8];
    int freqs[xSize * ySize * 8];
    int len = 0;

    enum transform curTransform;

    for (unsigned int x = xSize-1; x != -1; x--) {
        for (unsigned int y = ySize-1; y != -1; y--) {
            curTransform = IN_PLACE;
            while (curTransform != INVALID_TRANSFORM) {
                char * curStr = malloc(sizeof(char) * size * size);
                int curPos = 0;
                for (unsigned int yShift = 0; yShift < size; yShift++) {
                    for (unsigned int xShift = 0; xShift < size; xShift++) {
                        switch(curTransform) {
                            case IN_PLACE:
                                curStr[curPos++] = grLookup(grid, (x+xShift) % xSize, (y+yShift) % ySize );
                                break;
                            case ROT_90:
                                curStr[curPos++] = grLookup(grid, (x+(size-yShift)-1) % xSize, (y+xShift) % ySize );
                                break;
                            case ROT_180:
                                curStr[curPos++] = grLookup(grid, (x+(size-xShift)-1) % xSize, (y+(size-yShift)-1) % ySize );
                                break;
                            case ROT_270:
                                curStr[curPos++] = grLookup(grid, (x+yShift) % xSize, (y+(size-xShift)-1) % ySize );
                                break;
                            case REFL:
                                curStr[curPos++] = grLookup(grid, (x+yShift) % xSize, (y+xShift) % ySize );
                                break;
                            case REFL_ROT_90:
                                curStr[curPos++] = grLookup(grid, (x+(size-xShift)-1) % xSize, (y+yShift) % ySize );
                                break;
                            case REFL_ROT_180:
                                curStr[curPos++] = grLookup(grid, (x+(size-yShift)-1) % xSize, (y+(size-xShift)-1) % ySize );
                                break;
                            case REFL_ROT_270:
                                curStr[curPos++] = grLookup(grid, (x+xShift) % xSize, (y+(size-yShift)-1) % ySize );
                                break;
                            default:
                                assert(0 == 1);
                        }
                    }
                }

                for (int k = 0; k < len; k++) {
                    if (strncmp(curStr, allStrs[k], (size*size)) == 0) {
                        freqs[k]++;
                        free(curStr);
                        goto out;
                    }
                }
                freqs[len] = 1;
                allStrs[len++] = curStr;
                out:
                switch(curTransform) {
                    case IN_PLACE:
                        if (rotations){
                            curTransform++;
                        } else if (reflections) {
                            curTransform = REFL;
                        } else {
                            curTransform = INVALID_TRANSFORM;
                        }
                        break;
                    case ROT_270:
                        if (reflections) {
                            curTransform++;
                        } else {
                            curTransform = INVALID_TRANSFORM;
                        }
                        break;
                    case REFL:
                        if (rotations) {
                            curTransform++;
                        } else {
                            curTransform = INVALID_TRANSFORM;
                        }
                        break;
                    default:
                        curTransform++;
                }
            }
        }
    }

    for (int k = 0; k < len; k++) {
        Block block = blCreateFromString( size, allStrs[k] );
        block->freq = freqs[k];
        bsetAppend(self, block);
        free(allStrs[k]);
    }

    return self;
}


void bsetDestroy(BlockSet self) {
    if (self->locked) {
        bsetDestroyLocked(self);
    } else {
        bsetDestroyUnlocked(self);
    }
    free(self);
}

static void bsetDestroyUnlocked(BlockSet self) {
    while (self->blockList != NULL) {
        BlockListNode oldNode = self->blockList;
        self->blockList = self->blockList->next;
        blDestroy(oldNode->value);
        free(oldNode);
    }
}
static void bsetDestroyLocked(BlockSet self) {
    for (int k = self->len-1; k >= 0; k--) {
        bmDestroy(self->blocks[k]->overlapMasks[CARD_N]);
        bmDestroy(self->blocks[k]->overlapMasks[CARD_S]);
        bmDestroy(self->blocks[k]->overlapMasks[CARD_E]);
        bmDestroy(self->blocks[k]->overlapMasks[CARD_W]);
        blDestroy(self->blocks[k]);
    }
    bmDestroy(self->allTrueMask);
    free(self->blocks);
}

void bsetPrint(BlockSet self) {
    printf("Printing blockSet\n");
    if (self->locked) {
        bsetPrintLocked(self);
    } else {
        bsetPrintUnlocked(self);
    }
}

static void bsetPrintLocked(BlockSet self) {
    for (int k = 0; k < self->len; k++){
        Block curBlock = self->blocks[k];
        printf("BLOCK %d (freq %d):\n", absIndex(curBlock), curBlock->freq);
        blPrint(curBlock);
        #if DPRINT_LOCKED_DATA
            blPrintData(curBlock);
        #endif
        printf("\n");
    }

}
static void bsetPrintUnlocked(BlockSet self) {
    BlockListNode curNode = self->blockList;
    while (curNode != NULL) {
        printf("BLOCK (freq %d):\n", curNode->value->freq);
        blPrint(curNode->value);
        curNode = curNode->next;
        printf("\n");
    }
}

void bsetAppend(BlockSet self, Block block) {
    assert(!self->locked);

    BlockListNode newNode = malloc(sizeof(struct blockListNode));
    newNode->next = self->blockList;
    newNode->value = block;
    self->blockList = newNode;

    self->len += 1;
}

void bsetAppendFromString(BlockSet self, char * str) {
    assert(!self->locked);

    BlockListNode cur = self->blockList;
    while (cur != NULL) {
        if (strncmp(cur->value->values, str, (self->size*self->size)) == 0) {
            cur->value->freq++;
            return;
        }
        cur = cur->next;
    }
    bsetAppend(self, blCreateFromString(self->size, str));
}

void bsetLock(BlockSet self) {
    assert(!self->locked);
    assert(self->len > 0);

    self->locked = 1;
    self->blocks = malloc(sizeof(Block) * self->len);

    unsigned int absIndex = 0;

    Block curBlock, innerBlock;

    unsigned char numFields = ((self->len-1) / FIELD_LEN) + 1;
    self->allTrueMask = bmCreate(numFields);
    bmClear(self->allTrueMask);

    while (self->blockList != NULL) {
        curBlock = self->blockList->value;

        curBlock->logFreq = curBlock->freq * log2(curBlock->freq);

        curBlock->fieldIndex = absIndex / FIELD_LEN;
        curBlock->bitIndex = absIndex % FIELD_LEN;
//        printf("FieldLen is %d, absIndex is %d, bitINdex is %d\n", FIELD_LEN, absIndex, curBlock->bitIndex);
        curBlock->localMask = 1ULL << curBlock->bitIndex;
//        printf("Shifted is %llu | mask:", curBlock->localMask);
//        bmFieldPrint(curBlock->localMask);
//        printf("\n");

        blbmAdd(curBlock, self->allTrueMask);

        curBlock->overlapMasks[CARD_N] = bmCreate(numFields);
        curBlock->overlapMasks[CARD_S] = bmCreate(numFields);
        curBlock->overlapMasks[CARD_E] = bmCreate(numFields);
        curBlock->overlapMasks[CARD_W] = bmCreate(numFields);

        bmClear(curBlock->overlapMasks[CARD_N]);
        bmClear(curBlock->overlapMasks[CARD_S]);
        bmClear(curBlock->overlapMasks[CARD_E]);
        bmClear(curBlock->overlapMasks[CARD_W]);

        blMarkOverlaps(curBlock, curBlock, CARD_N);
        blMarkOverlaps(curBlock, curBlock, CARD_S);
        blMarkOverlaps(curBlock, curBlock, CARD_E);
        blMarkOverlaps(curBlock, curBlock, CARD_W);

        for (int innerIndex = absIndex-1; innerIndex >= 0; innerIndex--) {
            innerBlock = self->blocks[innerIndex];

            blMarkOverlaps(curBlock, innerBlock, CARD_N);
            blMarkOverlaps(curBlock, innerBlock, CARD_S);
            blMarkOverlaps(curBlock, innerBlock, CARD_E);
            blMarkOverlaps(curBlock, innerBlock, CARD_W);

            blMarkOverlaps(innerBlock, curBlock, CARD_N);
            blMarkOverlaps(innerBlock, curBlock, CARD_S);
            blMarkOverlaps(innerBlock, curBlock, CARD_E);
            blMarkOverlaps(innerBlock, curBlock, CARD_W);
        }

        self->blocks[absIndex] = curBlock;
        absIndex += 1;

        BlockListNode oldNode = self->blockList;
        self->blockList = self->blockList->next;
        free(oldNode);
    }
}

unsigned int bsetLen(BlockSet self) {
    return self->len;
}
Block bsetLookup(BlockSet self, unsigned int blockID) {
    return self->blocks[blockID];
}

Block bsetRandom(BlockSet self, Bitmask mask, int roll){
//    printf("   Randomly rolling from blockSet (roll %d)!\n", roll);

    unsigned int cherrypickValues[self->len+1];
    Bitmask newmask = nbmCopy(mask);
    bmFastCherrypick(newmask, cherrypickValues);
    bmDestroy(newmask);

//    printf("cherrypickValues is:");

//    for (int k = cherrypickValues[0]; k > 0; k--) {
//        printf("%d,", cherrypickValues[k]);
//    }
//    printf("\n");
    for (int k = cherrypickValues[0]; k > 0; k--) {
        Block block = self->blocks[cherrypickValues[k]];
//        printf("   Got block w/freq %d!\n", block->freq);
//        blPrint(block);

        roll -= block->freq;
        if (roll < 0) {
//            printf("   (it was selected!)\n");
            return block;
        }

//        printf("   Roll now %d!\n", roll);
    }
    assert(1==0);
    return NULL;
}
void bsetEntropy(BlockSet self, Bitmask bm, unsigned int * freq, float * entropy) {
#if DEBUG_BENCH
    float startTime = clock();
#endif

    int distinctBlocks = 0;
    float innerSum = 0;
    *freq = 0;

//    printf ("Setting entropy of bitmask %p / value:", bm);
//    bmPrint(bm);
//    printf("\n");

    for (int k = self->len - 1; k >= 0; k--) {
        Block block = bsetLookup(self, k);
//        printf ("    Ablock || bitIndex %u / fieldIndex %u || fieldMask:", block->bitIndex, block->fieldIndex);
//        bmFieldPrint(block->localMask);
//        printf("\n");

        if (blbmContains(block, bm)) {
            distinctBlocks++;
            *freq += block->freq;
            innerSum += block->logFreq;
//            innerSum += block->freq * log2(block->freq);
//            printf ("    blockFreq %d, total %d || bitIndex %u / fieldIndex %u || fieldMask:", *freq, block->freq, block->bitIndex, block->fieldIndex);

//            Bitmask t = bmCreate(bm->len);
//            blbmAdd(block, t);
//            bmFieldPrint(block->localMask);
//            printf("\n");
        }
    }
    if (distinctBlocks == 1) {
        *entropy = 0;
    } else {
        *entropy = log2(*freq) - (innerSum / *freq);
    }

//    printf ("Set final freq %d / entropy %f\n", *freq, *entropy);
#if DEBUG_BENCH
    bEntropyTime += clock() - startTime;
#endif
}

char bsetBlockToValue(BlockSet self, Bitmask bm){
    char bValue = -1;
    for (int k = self->len - 1; k >= 0; k--) {
        Block block = bsetLookup(self, k);
        if (blbmContains(block, bm)) {
            if (bValue == -1) {
                bValue = block->values[0];
            } else {
                return -1;
            }
        }
    }
    return bValue;
}

Bitmask bsetTrueMask(BlockSet self){
    return nbmCopy(self->allTrueMask);
}
Bitmask bsetFalseMask(BlockSet self){
    Bitmask bm = bmCreate( ((self->len-1) / FIELD_LEN) + 1 );
    bmClear(bm);
    return bm;
}

Bitmask bsetInverseValueMask(BlockSet self, char value) {
    Bitmask bm = bsetFalseMask(self);
    for (int k = 0; k < self->len; k++) {
        Block cur = bsetLookup(self, k);
        if (cur->values[0] != value) {
            blbmAdd(cur, bm);
        }
    }
    return bm;
}

int bsetTestSymmetry(BlockSet self) {
    for (int k = 0; k < self->len; k++) {
        Block cur = bsetLookup(self, k);
        for (int j = 0; j <= k; j++) {
            Block other = bsetLookup(self, j);

            printf ("Checking symmetry of block %d and block %d\n", k, j);
            if (blbmContains(cur, other->overlapMasks[CARD_N]) != blbmContains(other, cur->overlapMasks[CARD_S]) ) {
                printf ("Symmetry error!\n");
                return 0;
            }

            if (blbmContains(cur, other->overlapMasks[CARD_S]) != blbmContains(other, cur->overlapMasks[CARD_N]) ) {
                printf ("Symmetry error!\n");
                return 0;
            }

            if (blbmContains(cur, other->overlapMasks[CARD_E]) != blbmContains(other, cur->overlapMasks[CARD_W]) ) {
                printf ("Symmetry error!\n");
                return 0;

            }
            if (blbmContains(cur, other->overlapMasks[CARD_W]) != blbmContains(other, cur->overlapMasks[CARD_E]) ) {
                printf ("Symmetry error!\n");
                return 0;
            }
        }
    }
    return 1;
}
