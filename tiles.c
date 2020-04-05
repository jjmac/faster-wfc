#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"

Context coCreate(unsigned short xSize, unsigned short ySize) {
    Context self = malloc(sizeof(struct context));
    self->xSize = xSize;
    self->ySize = ySize;
    self->tiles = malloc(sizeof(tile) * (xSize * ySize));
    self->eHeap = malloc(sizeof(unsigned int) * (xSize * ySize + 1));
    assert(self->tiles != NULL);
    return self;
}

void coDestroy(Context self) {
    for (unsigned int k = self->xSize * self->ySize - 1; k >= 0; k--) {
        bmDestroy(self->tiles[k].validBlockMask);
        bmDestroy(self->tiles[k].rippleDifference);
    }
    free(self->tiles);
    free(self->eHeap);
    free(self);
}
void coPrint(Context self) {
    printf("Context:\n");
    for (unsigned int y = 0; y < self->ySize; y++) {
        putchar(' ');
        for (unsigned int x = 0; x < self->xSize; x++) {
            if (self->tiles[y*self->ySize + x].heapIndex == 0){
                putchar(self->tiles[y*self->ySize + x].value);
            } else {
                putchar('?');
            }
        }
        putchar('\n');
    }
}

void coHeappush(Context self, unsigned int tID) {
    unsigned int * eHeap = self->eHeap;
    tile * tiles = self->tiles;

    unsigned int curIndex = ++eHeap[0];
    unsigned int nextIndex = curIndex >> 1;

    float entropy = self->tiles[tID].entropy;

    while(nextIndex > 0) {
        if (tiles[eHeap[nextIndex]].entropy < entropy) {
            tiles[eHeap[nextIndex]].heapIndex = curIndex;
            eHeap[curIndex] = eHeap[nextIndex];
            curIndex = nextIndex;
            nextIndex = nextIndex >> 1;
        } else {
            break;
        }
    }
    eHeap[curIndex] = tID;
    self->tiles[tID].heapIndex = curIndex;
}
unsigned int coHeappop(Context self) {
    unsigned int * eHeap = self->eHeap;
    tile * tiles = self->tiles;

    unsigned int retVal = eHeap[1];

    unsigned int curIndex = 1;
    unsigned int lIndex = 2;
    unsigned int rIndex = 3;

    while(1) {
        if (lIndex == eHeap[0]) {
            eHeap[curIndex] = eHeap[lIndex];
            break;
        } else if (lIndex > eHeap[0]) {
            break;
        }
        if (tiles[eHeap[lIndex]].entropy < tiles[eHeap[rIndex]].entropy) {
            eHeap[curIndex] = eHeap[lIndex];
            curIndex = lIndex;
        } else {
            eHeap[curIndex] = eHeap[rIndex];
            curIndex = rIndex;
        }
        lIndex = curIndex*2;
        rIndex = lIndex+1;
    }
    eHeap[0]--;
    return retVal;
}

void coHeaprefresh(Context self, unsigned int tID) {
    assert (0);
}

void tiRefreshValues(Context self, BlockSet bset, unsigned int tID) {
    bsetEntropy(bset, self->tiles[tID].validBlockMask,  &(self->tiles[tID].freq), &(self->tiles[tID].entropy));
}

void tiCollapseTo(Context self, unsigned int tID, Block block) {
    Bitmask difference = self->tiles[tID].validBlockMask;

    /*
    printf("   Before edits, VBM is %p / value is:", self->tiles[tID].validBlockMask);
    bmPrint(self->tiles[tID].validBlockMask);
    printf("\n");
    printf("   Before edits, RipD is %p / value is:", self->tiles[tID].rippleDifference);
    bmPrint(self->tiles[tID].rippleDifference);
    printf("\n");
    */

    self->tiles[tID].validBlockMask = self->tiles[tID].rippleDifference;
    self->tiles[tID].rippleDifference = difference;

    blbmSet(block, self->tiles[tID].validBlockMask);
    blbmRemove(block, self->tiles[tID].rippleDifference);

    /*
    printf("   After edits, VBM is %p / value is:", self->tiles[tID].validBlockMask);
    bmPrint(self->tiles[tID].validBlockMask);
    printf("\n");
    printf("   After edits, RipD is %p / value is:", self->tiles[tID].rippleDifference);
    bmPrint(self->tiles[tID].rippleDifference);
    printf("\n");
    */
}
