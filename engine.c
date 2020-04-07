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

#define xTileID(tID) (tID % context->xSize)
#define yTileID(tID) (tID / context->xSize)

struct engine {
    BlockSet bset;
    Context context;
    int rSeed;

    unsigned int * changedTileIDs;
};

static int advance(Engine self);
static int rippleChangesFrom(Engine self, unsigned int tID);
static void addChangedTile(Engine self, unsigned int ctID);

Engine enCreate(BlockSet bset, Context context, int rSeed) {
    Engine self = malloc(sizeof(struct engine));
    self->bset = bset;
    self->context = context;
    self->rSeed = rSeed;
    return self;
}
void enDestroy(Engine self) {
    free(self);
}

void enRun(Engine self) {
    printf("=== Starting run!\n");
    enPrepare(self);
    printf("=== Prepare call complete!\n");
    enCoreLoop(self);
    printf("=== Core loop complete!\n");
    enCleanup(self);
}

void enPrepare(Engine self) {
    Context context = self->context;
    BlockSet bset = self->bset;

    printf("= In prepare!\n");

    for (unsigned int tID = context->xSize*context->ySize - 1; tID != -1; tID--) {
        context->tiles[tID].validBlockMask = bsetTrueMask(bset);
        context->tiles[tID].rippleDifference = bsetFalseMask(bset);
        context->tiles[tID].ctIndex = 0;
        tiRefreshValues(context, bset, tID);

//        printf("Tile %d has freq %d ent %f mask:", tID, context->tiles[tID].freq, context->tiles[tID].entropy);
//        bmPrint(context->tiles[tID].validBlockMask);
//        printf("\n");

        coHeapPush(context, tID);
    }

    self->changedTileIDs = malloc(sizeof(unsigned int) * (context->xSize * context->ySize + 1));

    srand(self->rSeed);
}

void enCoreLoop(Engine self) {
    while (self->context->eHeap[0]) {
        printf("= In core loop - advancing (%d tiles remain)!\n", self->context->eHeap[0]);
//        coHeapPrint(self->context);
        advance(self);
    }
}

void enCleanup(Engine self) {
    Context context = self->context;
    BlockSet bset = self->bset;

    for (unsigned int tID = 0; tID < context->xSize*context->ySize; tID++) {
        context->tiles[tID].value = bsetBlockToValue(bset, context->tiles[tID].validBlockMask);
        putchar(context->tiles[tID].value);
        if ((tID+1) % context->xSize == 0) {
            putchar('\n');
        }
    }
}

static int advance(Engine self) {
//    printf("In call to advance()!\n");

    // pick a tile to collapse
    unsigned int tID = coHeapPop(self->context);

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

    if (rippleChangesFrom(self, tID)) {
//        printf(" Finished rippleChangesFrom() - now refreshing freq/entropy values!\n");
//        printf(" Got %d changed tiles: ", self->changedTileIDs[0]);
//        for (int k = 1; k <= self->changedTileIDs[0]; k++) {
//            printf("%d, ", self->changedTileIDs[k]);
//        }
//        printf("\n");

//        printf("Performing initial heapPrint\n");
//        coHeapPrint(self->context);

        for (int k = 1; k <= self->changedTileIDs[0]; k++) {
//            printf("-- dealing with changed tile %d\n", self->changedTileIDs[k]);
            self->context->tiles[self->changedTileIDs[k]].ctIndex = 0;
            tiHeapRefresh(self->context, self->bset, self->changedTileIDs[k]);
//            coHeapPrint(self->context);
        }
//        coHeapPrint(self->context);
//        printf("-- autochanged tile %d\n", tID);
        tiRefreshValues(self->context, self->bset, tID);
        self->context->tiles[tID].ctIndex = 0;

        self->changedTileIDs[0] = 0;
        return 1;
    } else {
//        printf(" Ripple changes failed!\n");
        assert(0);
        return 0;
    }
}

// TODO: Rewrite this function so that it weaves a linkedlist through context->tiles rather than constructing a paralell list!
typedef struct visitNode * VisitNode;
struct visitNode {
    unsigned int tID;
    VisitNode next;
};

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

        toVisit = toVisit->next;

//        printf("   - Visiting tile (%d, %d) with vbm:", xTileID(tID), yTileID(tID));
//        bmPrint(curTile->validBlockMask);
//        printf("  (falisty %d)\n", bmFalse(curTile->validBlockMask));
//        printf("   - Tile also has rippleDiff:");
//        bmPrint(curDifference);
//        printf("\n");
        if (bmFalse(curTile->validBlockMask)) {
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

//            printf("    Blocks being removed are:");
//            while (index > 0) {
//                printf("%d,", diffBlockIDs[index] );
//                index--;
//            }
//            printf("\n");

            index = diffBlockIDs[0];

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
    return 1;
}
