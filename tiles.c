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

#define CHECK_HEAPS 0

#define xTileID(tID) (tID % self->xSize)
#define yTileID(tID) (tID / self->xSize)

float bHeapPushTime = 0;
float bHeapRemoveTime = 0;
float bHeapRefreshTime = 0;
float bHeapRefreshValuesTime = 0;
int bHeapRefreshCalls = 0;
float bSiftDownTime = 0;

static void innerHeapRemove(Context self, unsigned int curIndex);
static void heapSiftDown(Context self, unsigned int curIndex);

Context coCreate(unsigned short xSize, unsigned short ySize) {
    Context self = malloc(sizeof(struct context));
    self->xSize = xSize;
    self->ySize = ySize;
    self->tiles = malloc(sizeof(tile) * (xSize * ySize));
    self->eHeap = malloc(sizeof(unsigned int) * (xSize * ySize + 1));
    self->prepared = 0;
    assert(self->tiles != NULL);
    return self;
}

void coDestroy(Context self) {
    if (self->prepared) {
        for (unsigned int k = self->xSize * self->ySize - 1; k != -1; k--) {
//            printf("Destroying context - k is %d, tiles[k] is %p\n", k, self->tiles[k].validBlockMask);
            bmDestroy(self->tiles[k].validBlockMask);
            bmDestroy(self->tiles[k].rippleDifference);
        }
    }
    free(self->tiles);
    free(self->eHeap);
    free(self);
}

int coPrepare(Context self, BlockSet bset) {
    int contradicted = 0;

    self->prepared = 1;

    self->eHeap[0] = 0;
    self->toCollapse = self->xSize*self->ySize;
    for (unsigned int tID = self->toCollapse-1; tID != -1; tID--) {
        int tileDirs = ALL_CARDS;
        if (yTileID(tID) == 0) {
            tileDirs ^= cardBit(CARD_S);
        }
        if (yTileID(tID) == self->ySize-1){
            tileDirs ^= cardBit(CARD_N);
        }
        if (xTileID(tID) == 0) {
            tileDirs ^= cardBit(CARD_E);
        }
        if (xTileID(tID) == self->xSize-1){
            tileDirs ^= cardBit(CARD_W);
        }

        self->tiles[tID].validBlockMask = bsetPlacableMask(bset, tileDirs);
        self->tiles[tID].rippleDifference = bsetFalseMask(bset);
        self->tiles[tID].ctIndex = 0;
        self->tiles[tID].heapIndex = 0;

        /*
        printf("tID, %d, (%d, %d) has tileDirs %d | vbm:", tID, xTileID(tID), yTileID(tID), tileDirs);
        bmPrint(self->tiles[tID].validBlockMask);
        printf("\n");
        */

        if (bmFalse(self->tiles[tID].validBlockMask)) {
            contradicted = 1;
        }

        tiRefreshValues(self, bset, tID);
        if (self->tiles[tID].entropy <= 0) {
            self->toCollapse--;
        }
    }

    if (contradicted) {
        printf("Incomplete or contradicted blockset - no valid tiling possible!\n");
        return 0;
    } else {
        return 1;
    }
}

Context coCopy(Context self) {
    assert(self->prepared);
    Context other = coCreate(self->xSize, self->ySize);

    other->prepared = 1;
    for (unsigned int hIndex = self->eHeap[0]; hIndex != -1; hIndex--) {
        other->eHeap[hIndex] = self->eHeap[hIndex];
    }
    other->toCollapse = self->toCollapse;
    for (unsigned int tID = self->xSize*self->ySize-1; tID != -1; tID--) {
        other->tiles[tID].validBlockMask = nbmCopy(self->tiles[tID].validBlockMask);
        other->tiles[tID].rippleDifference = nbmCopy(self->tiles[tID].rippleDifference);
        other->tiles[tID].ctIndex = self->tiles[tID].ctIndex;
        other->tiles[tID].heapIndex = self->tiles[tID].heapIndex;

        other->tiles[tID].freq = self->tiles[tID].freq;
        other->tiles[tID].entropy = self->tiles[tID].entropy;
        other->tiles[tID].value = self->tiles[tID].value;
    }
    return other;
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
#if CHECK_HEAPS
//    printf("      Sanity-checking heap with %d elements\n", self->eHeap[0]);
    for (int k = 1; k <= self->eHeap[0]; k++) {
        tile t = self->tiles[self->eHeap[k]];
        if (t.heapIndex != k) {
            coHeapPrint(self);
            printf("      Sanity-check failed on element index %d\n", k);
            assert(0);
        }
    }
#endif
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

//    if (self->tiles[tID].entropy <= 0) {
//        coHeapPrint(self);
//        printf("!!!!! Tried to remove tile %d from heap\n", tID);
//        assert (1 == 0);
//    }

#if DEBUG_BENCH
    bHeapRefreshValuesTime += clock() - startTime;
#endif

    tiRefreshValues(self, bset, tID);

//    printf("      Refreshing tile %d - after refresh, entropy is %f!\n", tID, self->tiles[tID].entropy);
    if (self->tiles[tID].entropy <= 0) {
//        printf("      Consequently removing tile %d from heap!\n", tID);
        coHeapRemove(self, tID);
        self->tiles[tID].heapIndex = 0;
        self->toCollapse--;
    } else {
//        printf("      Consequently sifting tile %d around in heap!\n", tID);
        heapSiftDown(self, self->tiles[tID].heapIndex);
    }

#if DEBUG_BENCH
    bHeapRefreshTime += clock() - startTime;
    bHeapRefreshCalls++;
#endif
}


void coHeapRemove(Context self, unsigned int tID) {
#if DEBUG_BENCH
    float startTime = clock();
#endif
//    printf("      Removing tile %d from heap with %d elements!\n", tID, self->eHeap[0]);
    innerHeapRemove(self, self->tiles[tID].heapIndex);

    heapSanityCheck(self);
#if DEBUG_BENCH
    bHeapRemoveTime += clock() - startTime;
#endif
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
