#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cardinals.h"
#include "grids.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"

#include "benchmarking.h"

#define xTileID(tID) (tID % context->xSize)
#define yTileID(tID) (tID / context->xSize)

float bCoreLoopTime = 0;
float bRippleTime = 0;

struct engine {
    BlockSet bset;
    Context context;

    unsigned int * changedTileIDs;
};

// TODO: Rewrite this function so that it weaves a linkedlist through context->tiles rather than constructing a paralell list!
typedef struct visitNode * VisitNode;
struct visitNode {
    unsigned int tID;
    VisitNode next;
};

static int enCoreLoop(Engine self, int toCollapseThreshold);
static int advance(Engine self);
static int initialRippleChangesFrom(Engine self, unsigned int tID);
static int rippleChangesFrom(Engine self, unsigned int tID);
static int processChildTile(Engine self, unsigned int tID, cardinal dir, Bitmask cDifference, VisitNode * toVisit);
static void addChangedTile(Engine self, unsigned int ctID);

Engine enCreate(BlockSet bset, int xSize, int ySize) {
    Engine self = malloc(sizeof(struct engine));
    self->bset = bset;
    self->context = coCreate(xSize, ySize);
    self->changedTileIDs = malloc(sizeof(unsigned int) * (xSize*ySize + 1));
    return self;
}
void enDestroy(Engine self) {
    coDestroy(self->context);
    free(self->changedTileIDs);
    free(self);
}

int enPrepare(Engine self, int rSeed) {
    srand(rSeed);
    return coPrepare(self->context, self->bset);
}

int enRun(Engine self, int rSeed) {
//    printf("=== Starting run!\n");
    if (!enPrepare(self, rSeed)) {
        return 0;
    }
//    printf("=== Prepare call complete!\n");
    if (!enRecursiveCoreLoop(self, 10, 1000)) {
        printf("=== Contradiction error - exiting!\n");
        return 0;
    }
//    printf("=== Core loop complete!\n");
    enCleanup(self);
    return 1;
}

static int enCoreLoop(Engine self, int toCollapseThreshold) {
#if DEBUG_BENCH
    float startTime = clock();
#endif

    if (toCollapseThreshold < 0) {
        toCollapseThreshold = 0;
    }

//    printf("= In core loop - (%d tiles remain, threshold %d)!\n", self->context->toCollapse, toCollapseThreshold);

    while (self->context->toCollapse > toCollapseThreshold) {
//        printf("= In core loop - advancing (%d tiles remain, %d in heap)!\n", self->context->toCollapse, self->context->eHeap[0]);
//        coHeapPrint(self->context);
        if (!advance(self)) {
            return 0;
        }
    }
#if DEBUG_BENCH
    bCoreLoopTime += clock() - startTime;
#endif
    return 1;
}

int enRecursiveCoreLoop(Engine self, int maxContradictions, int checkpointInterval) {
    if (self->context->toCollapse == 0) {
        return 1;
    }

    Context backupContext = coCopy(self->context);

    int origMaxContradictions = maxContradictions;

    while (maxContradictions > 0) {
//        printf("Calling core loop with %d to collapse (%d in heap)\n", self->context->toCollapse, self->context->eHeap[0]);
        if (enCoreLoop(self, self->context->toCollapse - checkpointInterval)) {
            if (enRecursiveCoreLoop(self, origMaxContradictions, checkpointInterval)) {
                coDestroy(backupContext);
                return 1;
            }
        }
        maxContradictions--;
//        printf("State is:\n");
//        enPrint(self);
//        printf("Contradiction error: %d attempts left (toCollapse is %d)\n", maxContradictions, self->context->toCollapse);
        coDestroy(self->context);
        self->context = coCopy(backupContext);
    }
    return 0;
}

void enCleanup(Engine self) {
//    coDestroy(self->context);
//    enPrint(self);
}

void enWriteToBuffer(Engine self, char * buffer) {
    Context context = self->context;
    BlockSet bset = self->bset;
    for (unsigned int tID = 0; tID < context->xSize*context->ySize; tID++) {
        buffer[tID] = bsetBlockToValue(bset, context->tiles[tID].validBlockMask);
    }
}

void enPrint(Engine self) {
    Context context = self->context;
    BlockSet bset = self->bset;
    for (unsigned int tID = 0; tID < context->xSize*context->ySize; tID++) {
        context->tiles[tID].value = bsetBlockToValue(bset, context->tiles[tID].validBlockMask);
        putchar(context->tiles[tID].value);

//        printf("%d", tID);
        if ((tID+1) % context->xSize == 0) {
            putchar('\n');
        }
    }
}

int enCoerceXY(Engine self, unsigned int x, unsigned int y, char value){
    return enCoerceTile(self, (y*self->context->xSize)+x, value);
}

int enCoerceTile(Engine self, unsigned int tID, char value) {
    Context context = self->context;
    BlockSet bset = self->bset;

    Bitmask inverseValueMask = bsetInverseValueMask(bset, value);

    bmXor(context->tiles[tID].validBlockMask, inverseValueMask);
    bmOr(context->tiles[tID].rippleDifference, inverseValueMask);

    bmDestroy(inverseValueMask);

    return initialRippleChangesFrom(self, tID);
}

static int advance(Engine self) {
//    printf("In call to advance()!\n");
    Context context = self->context;
    unsigned int tID = 0;

/*    printf("Heap is:\n");
    coHeapPrint(self->context);
    printf("\n");*/

    // pick a tile to collapse
    if (self->context->eHeap[0] > 0) {
        tID = coHeapPop(self->context);
    } else {
        for (tID = context->xSize * context->ySize - 1; tID != -1; tID--) {
            if (context->tiles[tID].heapIndex == 0 && context->tiles[tID].entropy > 0) {
                break;
            }
        }
    }
    if (tID == -1) {
        assert(0);
    }

//    printf(" Got random tID: %d\n", tID);

//    printf(" Tile blockmask is %p, freq %d\n", self->context->tiles[tID].validBlockMask, self->context->tiles[tID].freq);

//    coHeapPrint(self->context);

    // pick a block to collapse it to
    Block block = bsetRandom(self->bset, self->context->tiles[tID].validBlockMask, rand() % (self->context->tiles[tID].freq));

//    printf(" Got random blockPtr %p\n", block);
//    printf(" Collapsing tile (%d, %d) to block:\n", xTileID(tID), yTileID(tID));
//    blPrint(block);

    tiCollapseTo(self->context, tID, block);
//    printf(" Done tile collapse!\n");

    if (initialRippleChangesFrom(self, tID)) {
        return 1;
    } else {
//        printf(" Ripple changes failed!\n");
        return 0;
    }
}


static int initialRippleChangesFrom(Engine self, unsigned int tID) {
    self->changedTileIDs[0] = 0;
    addChangedTile(self, tID);

    if (rippleChangesFrom(self, tID)) {

/*
        printf(" Finished rippleChangesFrom() - now refreshing freq/entropy values!\n");
        printf(" Got %d changed tiles: ", self->changedTileIDs[0]);
        for (int k = 1; k <= self->changedTileIDs[0]; k++) {
            printf("%d, ", self->changedTileIDs[k]);
        }
        printf("\n");
*/

        for (int k = 1; k <= self->changedTileIDs[0]; k++) {
//            printf("-- dealing with changed tile %d w entropy %e\n", self->changedTileIDs[k], self->context->tiles[self->changedTileIDs[k]].entropy);
//            coHeapPrint(self->context);

            self->context->tiles[self->changedTileIDs[k]].ctIndex = 0;
            if (self->context->tiles[self->changedTileIDs[k]].heapIndex != 0) {
//                printf("  -- tiHeapRefresh\n");
                tiHeapRefresh(self->context, self->bset, self->changedTileIDs[k]);
            } else {
//                printf("  -- tiRefreshValues\n");
                tiRefreshValues(self->context, self->bset, self->changedTileIDs[k]);
                if (self->context->tiles[self->changedTileIDs[k]].entropy > 0) {
//                    printf("  -- coHeapPush %e vs\n", self->context->tiles[self->changedTileIDs[k]].entropy);
                    coHeapPush(self->context, self->changedTileIDs[k]);
                } else {
                    self->context->toCollapse--;
                }
            }
//            coHeapPrint(self->context);
//            printf("-- done dealing with tile %d\n", k);
        }
//        coHeapPrint(self->context);

//        tiRefreshValues(self->context, self->bset, tID);
//        self->context->tiles[tID].ctIndex = 0;
        return 1;
    } else {
//        printf(" Ripple changes failed!\n");
        return 0;
    }
}


static void addChangedTile(Engine self, unsigned int ctID) {
//    printf ("      Adding changed tile %d (%d, %d)\n", ctID, xTileID(ctID), yTileID(ctID));
    if (self->context->tiles[ctID].ctIndex == 0) {
        self->context->tiles[ctID].ctIndex = self->changedTileIDs[0];
        self->changedTileIDs[0]++;
        self->changedTileIDs[self->changedTileIDs[0]] = ctID;
//        printf ("      Tile %d now in heap at %d\n", ctID, self->context->tiles[ctID].ctIndex);
    } else {
//        printf ("      Tile %d was already in heap at %d - not adding\n", ctID, self->context->tiles[ctID].ctIndex);
    }
}

static int rippleChangesFrom(Engine self, unsigned int tID) {
#if DEBUG_BENCH
    float startTime = clock();
#endif
//    printf("   In call to rippleChangesFrom()!\n");

    Context context = self->context;
    BlockSet bset = self->bset;
    unsigned int diffBlockIDs[context->tiles[0].validBlockMask->len * FIELD_LEN + 1];

    tile * curTile = NULL;
    Bitmask curDifference = NULL;

    VisitNode toVisit = malloc(sizeof(struct visitNode));
    toVisit->tID = tID;
    toVisit->next = NULL;

    while (toVisit != NULL) {
        tID = toVisit->tID;
        curTile = &(context->tiles[tID]);
        curDifference = curTile->rippleDifference;

        VisitNode oldToVisit = toVisit;
        toVisit = toVisit->next;
        free(oldToVisit);

/*        printf("   - Visiting tile (%d, %d) with vbm:", xTileID(tID), yTileID(tID));
        bmPrint(curTile->validBlockMask);
        printf("  (falisty %d)\n", bmFalse(curTile->validBlockMask));
        printf("   - Tile also has rippleDiff:");
        bmPrint(curDifference);
        printf("\n");
        */
        if (bmFalse(curTile->validBlockMask)) {
            while (toVisit != NULL) {
                oldToVisit = toVisit;
                toVisit = toVisit->next;
                free(oldToVisit);
            }
            return 0;
        } else {
            Bitmask nDifference = bmCreate(curDifference->len);
            Bitmask sDifference = bmCreate(curDifference->len);
            Bitmask eDifference = bmCreate(curDifference->len);
            Bitmask wDifference = bmCreate(curDifference->len);

            bmClear(nDifference);
            bmClear(sDifference);
            bmClear(eDifference);
            bmClear(wDifference);

            bmFastCherrypick(curDifference, diffBlockIDs);
            unsigned int index = diffBlockIDs[0];

            /*
            printf("    Blocks being removed are:");
            while (index > 0) {
                printf("%d,", diffBlockIDs[index] );
                index--;
            }
            printf("\n");
            index = diffBlockIDs[0];
            */

            while (index > 0) {
                Block curBlock = bsetLookup(bset, diffBlockIDs[index--]);
//                printf("   - hitting the segfault, curBlock is %p\n", curBlock);

                bmOr(nDifference, curBlock->overlapMasks[CARD_S]);
                bmOr(sDifference, curBlock->overlapMasks[CARD_N]);
                bmOr(eDifference, curBlock->overlapMasks[CARD_W]);
                bmOr(wDifference, curBlock->overlapMasks[CARD_E]);

                /*
                printf("   - Block %d had enabled(n):", diffBlockIDs[index+1]);
                bmPrint(curBlock->overlapMasks[CARD_N]);
                printf("\n");
                printf("   - appending to sDifference:");
                bmPrint(curBlock->overlapMasks[CARD_S]);
                printf("\n");
                printf("   - Adjusting eDifference:");
                bmPrint(eDifference);
                printf("\n");
                printf("   - Adjusting wDifference:");
                bmPrint(wDifference);
                printf("\n");
                */

            }

            /*
            printf("   - Created nDifference:");
            bmPrint(nDifference);
            printf("\n");
            printf("   - Created sDifference:");
            bmPrint(sDifference);
            printf("\n");
            printf("   - Created eDifference:");
            bmPrint(eDifference);
            printf("\n");
            printf("   - Created wDifference:");
            bmPrint(wDifference);
            printf("\n");
            */

            processChildTile(self, tID, CARD_N, nDifference, &toVisit);
            processChildTile(self, tID, CARD_S, sDifference, &toVisit);
            processChildTile(self, tID, CARD_E, eDifference, &toVisit);
            processChildTile(self, tID, CARD_W, wDifference, &toVisit);

            bmDestroy(nDifference);
            bmDestroy(sDifference);
            bmDestroy(eDifference);
            bmDestroy(wDifference);
        }
    }
//    printf("   Leaving call to rippleChangesFrom()!\n");
#if DEBUG_BENCH
    bRippleTime += clock() - startTime;
#endif
    return 1;
}

static int processChildTile(Engine self, unsigned int tID, cardinal dir, Bitmask cDifference, VisitNode * toVisit) {
    Context context = self->context;
    BlockSet bset = self->bset;

    tile * curTile = &(context->tiles[tID]);

    unsigned int ctID;
    switch(dir) {
        case CARD_N:
            if (yTileID(tID) > 0) {
                ctID = tID - context->xSize;
                break;
            } else {
                return 0;
            }
        case CARD_S:
            if (yTileID(tID) < (context->ySize - 1)) {
                ctID = tID + context->xSize;
                break;
            } else {
                return 0;
            }
        case CARD_E:
            if (xTileID(tID) < (context->xSize - 1)) {
                ctID = tID + 1;
                break;
            } else {
                return 0;
            }
        case CARD_W:
            if (xTileID(tID) > 0) {
                ctID = tID - 1;
                break;
            } else {
                return 0;
            }
        default:
            assert(1==0);
    }

//    printf("      - In ChildTile (%d, %d) / p rmv:", xTileID(ctID), yTileID(ctID));
//    bmPrint(cDifference);
//    printf("\n");

    tile * cTile = &(context->tiles[ctID]);
    unsigned int diffBlockIDs[bsetLen(bset) + 1];

    cTile = &(context->tiles[ctID]);
//    printf("        - Adj sTile (%d, %d) with vbm:", xTileID(ctID), yTileID(ctID));
//    bmPrint(cTile->validBlockMask);
//    printf("\n");

    bmAnd(cDifference, cTile->validBlockMask);

//    printf("      - After bmAnd, cDifference is:");
//    bmPrint(cDifference);
//    printf("\n");

    bmFastCherrypick(cDifference, diffBlockIDs);
    unsigned int index = diffBlockIDs[0];

//    printf ("      - Printing diffBlockIDs:");
//    while (index > 0) {
//        printf("%d,", diffBlockIDs[index]);
//        index--;
//    }
//    printf("\n");
    index = diffBlockIDs[0];

//    printf("          - *Actual* diff blockIDs:");
    while (index > 0) {
//        printf("          - Would add(?) block %d\n", diffBlockIDs[index]);
        Block curBlock = bsetLookup(bset, diffBlockIDs[index--]);

        if (! bmAndValue(curBlock->overlapMasks[dir], curTile->validBlockMask ) ) {
//            printf("%d,", diffBlockIDs[index+1]);
            blbmAdd(curBlock, cDifference);
//            printf("          - Actually adding block %d, leaves cDiff as:", diffBlockIDs[index+1]);
//            bmPrint(cDifference);
//            printf("\n");
        }
    }
//    printf("\n");

    if (bmTrue(cDifference)) {
        VisitNode nextToVisit = malloc(sizeof(struct visitNode));
        nextToVisit->tID = ctID;
        nextToVisit->next = *toVisit;
        *toVisit = nextToVisit;

        addChangedTile(self, ctID);

        bmOr(cTile->rippleDifference, cDifference);
        bmNot(cDifference);
        bmAnd(cTile->validBlockMask, cDifference);
    }

//    printf("        - New tile (%d, %d) chngd vbm:", xTileID(ctID), yTileID(ctID));
//    bmPrint(cTile->validBlockMask);
//    printf("\n");

    return 1;
}
