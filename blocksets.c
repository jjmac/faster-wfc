#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"

#define DPRINT_LOCKED_DATA 1

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
        bmDestroy(oldNode->value->overlapMasks[CARD_N]);
        bmDestroy(oldNode->value->overlapMasks[CARD_S]);
        bmDestroy(oldNode->value->overlapMasks[CARD_E]);
        bmDestroy(oldNode->value->overlapMasks[CARD_W]);
        blDestroy(oldNode->value);
        free(oldNode);
    }
}
static void bsetDestroyLocked(BlockSet self) {
    for (int k = self->len; k >= 0; k--) {
        blDestroy(self->blocks[k]);
    }
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
        printf("BLOCK %d:\n", absIndex(curBlock));
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
        printf("BLOCK :\n");
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

void bsetLock(BlockSet self) {
    assert(!self->locked);
    assert(self->len > 0);

    self->locked = 1;
    self->blocks = malloc(sizeof(Block) * self->len);

    unsigned int absIndex = 0;

    Block curBlock, innerBlock;

    unsigned char numFields = ((self->len-1) / FIELD_LEN) + 1;
    self->allTrueMask = bmCreate(numFields);

    while (self->blockList != NULL) {
        curBlock = self->blockList->value;

        curBlock->freq = 1; // CHANGE

        curBlock->fieldIndex = absIndex / FIELD_LEN;
        curBlock->bitIndex = absIndex % FIELD_LEN;
        curBlock->localMask = 1ULL << (absIndex % 64);

        blbmAdd(curBlock, self->allTrueMask);

        curBlock->overlapMasks[CARD_N] = bmCreate(numFields);
        curBlock->overlapMasks[CARD_S] = bmCreate(numFields);
        curBlock->overlapMasks[CARD_E] = bmCreate(numFields);
        curBlock->overlapMasks[CARD_W] = bmCreate(numFields);

        blMarkOverlaps(curBlock, curBlock, CARD_N);
        blMarkOverlaps(curBlock, curBlock, CARD_W);
        blMarkOverlaps(curBlock, curBlock, CARD_E);
        blMarkOverlaps(curBlock, curBlock, CARD_S);

        for (int innerIndex = absIndex-1; innerIndex >= 0; innerIndex--) {
            innerBlock = self->blocks[innerIndex];

            blMarkOverlaps(curBlock, innerBlock, CARD_N);
            blMarkOverlaps(curBlock, innerBlock, CARD_W);
            blMarkOverlaps(curBlock, innerBlock, CARD_E);
            blMarkOverlaps(curBlock, innerBlock, CARD_S);

            blMarkOverlaps(innerBlock, curBlock, CARD_N);
            blMarkOverlaps(innerBlock, curBlock, CARD_W);
            blMarkOverlaps(innerBlock, curBlock, CARD_E);
            blMarkOverlaps(innerBlock, curBlock, CARD_S);
        }

        self->blocks[absIndex] = curBlock;
        absIndex += 1;

        BlockListNode oldNode = self->blockList;
        self->blockList = self->blockList->next;
        free(oldNode);
    }
}

Block bsetLookup(BlockSet self, unsigned int blockID) {
    return self->blocks[blockID];
}

Block bsetRandom(BlockSet self, Bitmask mask, int roll){
    printf("Randomly rolling from blockSet (roll %d)!\n", roll);

    unsigned int cherrypickValues[self->len+1];
    Bitmask newmask = nbmCopy(mask);
    bmFastCherrypick(newmask, cherrypickValues);
    bmDestroy(newmask);

    printf("Finished cherrypick!\n");

    for (int k = cherrypickValues[0]; k > 0; k--) {
        Block block = self->blocks[cherrypickValues[k]];
        printf("Got block w/freq %d!\n", block->freq);
        blPrint(block);

        roll -= block->freq;
        if (roll <= 0) {
            return block;
        }

        printf("Roll now %d!\n", roll);

    }
    assert(1==0);
    return NULL;
}
void bsetEntropy(BlockSet bset, Bitmask bm, unsigned int * freq, float * entropy) {
    float innerSum = 0;
    *freq = 0;

    printf ("Setting entropy of bitmask %p / value:", bm);
    bmPrint(bm);
    printf("\n");

    for (int k = bset->len - 1; k >= 0; k--) {
        Block block = bsetLookup(bset, k);
        if (blbmContains(block, bm)) {
            *freq += block->freq;
            innerSum += block->freq * log2(block->freq);
        }
    }
    *entropy = log2(*freq) - (innerSum / *freq);

    printf ("Set final freq %d / entropy %f:", *freq, *entropy);

}

Bitmask bsetTrueMask(BlockSet self){
    return nbmCopy(self->allTrueMask);
}
Bitmask bsetFalseMask(BlockSet self){
    return bmCreate( ((self->len-1) / FIELD_LEN) + 1 );
}
