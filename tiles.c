#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cardinals.h"
#include "grids.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"

#include "benchmarking.h"

float bHeapPushTime = 0;
float bHeapRefreshTime = 0;
float bSiftDownTime = 0;

static void innerHeapRemove(Context self, unsigned int curIndex);

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

static void heapSanityCheck(Context self){
//    printf("      Sanity-checking heap with %d elements\n", self->eHeap[0]);
    for (int k = 1; k <= self->eHeap[0]; k++) {
        tile t = self->tiles[self->eHeap[k]];
        if (t.heapIndex != k) {
            coHeapPrint(self);
            printf("      Sanity-check failed on element index %d\n", k);
            assert(0);
        }
    }
}

void coHeapPrint(Context self) {
    printf("      Printing context heap with %d elements!\n", self->eHeap[0]);
    for (int k = 1; k <= self->eHeap[0]; k++) {
        tile t = self->tiles[self->eHeap[k]];
        printf("      - H %d / %d | tile %d (%d, %d) | entropy %f\n", k, t.heapIndex, self->eHeap[k], self->eHeap[k] % self->xSize, self->eHeap[k] / self->xSize, t.entropy );
    }
}

void coHeapPush(Context self, unsigned int tID) {
#if DEBUG_BENCH
    float startTime = clock();
#endif
//    printf("      Pushing tile %d onto heap with %d elements!\n", tID, self->eHeap[0]);
    unsigned int * eHeap = self->eHeap;
    tile * tiles = self->tiles;

    unsigned int curIndex = ++eHeap[0];
    unsigned int nextIndex = curIndex >> 1;

    float entropy = self->tiles[tID].entropy;

    while(nextIndex > 0) {

//        printf("        -- In push, tile %d has entropy %f and is inserting at %d\n", tID, entropy, curIndex);

        if (tiles[eHeap[nextIndex]].entropy > entropy) {
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

    heapSanityCheck(self);
#if DEBUG_BENCH
    bHeapPushTime += clock() - startTime;
#endif
}

unsigned int coHeapPop(Context self) {
    unsigned int retVal = self->eHeap[1];
    innerHeapRemove(self, 1);

    heapSanityCheck(self);

    self->tiles[retVal].heapIndex = 0;
    return retVal;
}

void tiHeapRefresh(Context self, BlockSet bset, unsigned int tID) {
#if DEBUG_BENCH
    float startTime = clock();
#endif
    if (self->tiles[tID].entropy == 0) {
//        coHeapPrint(self);
        printf("!!!!! Tried to remove tile %d from heap\n", tID);
        assert (1 == 0);
    }
    coHeapRemove(self, tID);
//    coHeapPrint(self);
    tiRefreshValues(self, bset, tID);
    if (self->tiles[tID].entropy > 0) {
        coHeapPush(self, tID);
    }
#if DEBUG_BENCH
    bHeapRefreshTime += clock() - startTime;
#endif
}

void coHeapRemove(Context self, unsigned int tID) {
//    printf("      Removing tile %d from heap with %d elements!\n", tID, self->eHeap[0]);
    innerHeapRemove(self, self->tiles[tID].heapIndex);

    heapSanityCheck(self);
}

static void heapSiftDown(Context self, unsigned int curIndex) {
#if DEBUG_BENCH
    float startTime = clock();
#endif
    unsigned int * eHeap = self->eHeap;
    tile * tiles = self->tiles;

    unsigned int tID = eHeap[curIndex];
    float entropy = tiles[tID].entropy;

    while(1) {
        unsigned int lIndex = curIndex*2;
        unsigned int rIndex = lIndex+1;

        if (lIndex > eHeap[0]) {
            break;
        } else {
            if ( (lIndex == eHeap[0]) || (tiles[eHeap[lIndex]].entropy < tiles[eHeap[rIndex]].entropy) ) {
                if (tiles[eHeap[lIndex]].entropy < entropy) {
                    eHeap[curIndex] = eHeap[lIndex];
                    tiles[eHeap[curIndex]].heapIndex = curIndex;
                    curIndex = lIndex;
                } else {
                    break;
                }
            } else {
                if (tiles[eHeap[rIndex]].entropy < entropy) {
                    eHeap[curIndex] = eHeap[rIndex];
                    tiles[eHeap[curIndex]].heapIndex = curIndex;
                    curIndex = rIndex;
                } else {
                    break;
                }
            }
        }
    }
    tiles[tID].heapIndex = curIndex;
    eHeap[curIndex] = tID;

#if DEBUG_BENCH
    bSiftDownTime += clock() - startTime;
#endif
}

static void innerHeapRemove(Context self, unsigned int curIndex) {
    unsigned int * eHeap = self->eHeap;
    tile * tiles = self->tiles;

    eHeap[curIndex] = eHeap[eHeap[0]];
    tiles[eHeap[curIndex]].heapIndex = curIndex;
    eHeap[0]--;

    heapSiftDown(self, curIndex);
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
