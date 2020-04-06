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

struct engine {
    BlockSet bset;
    Context context;
    int rSeed;
};

static int advance(Engine self);
static int rippleChangesFrom(Engine self, unsigned int tID, unsigned int * changedTiles);

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
        tiRefreshValues(context, bset, tID);

        printf("Tile %d has freq %d ent %f mask:", tID, context->tiles[tID].freq, context->tiles[tID].entropy);
        bmPrint(context->tiles[tID].validBlockMask);
        printf("\n");

        coHeappush(context, tID);
    }

    srand(self->rSeed);
}

void enCoreLoop(Engine self) {
    while (self->context->eHeap[0]) {
        printf("= In core loop - advancing!\n");
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
    printf("In call to advance()!\n");

    // pick a tile to collapse
    unsigned int tID = coHeappop(self->context);

    printf(" Got random tID: %d\n", tID);

    printf(" Tile blockmask is %p, freq %d\n", self->context->tiles[tID].validBlockMask, self->context->tiles[tID].freq);

    // pick a block to collapse it to
    Block block = bsetRandom(self->bset, self->context->tiles[tID].validBlockMask, rand() % (self->context->tiles[tID].freq));

    printf(" Got random blockPtr %p\n", block);
    printf(" Collapsing tile %d to block:\n", tID);
    blPrint(block);

    tiCollapseTo(self->context, tID, block);
    printf(" Done tile collapse!\n");

    unsigned int * changedTiles = malloc(sizeof(unsigned int) * 900);
    if (rippleChangesFrom(self, tID, changedTiles)) {
        printf(" Finished rippleChangesFrom() - now refreshing freq/entropy values!\n");
        printf(" Got %d changed tiles: ", changedTiles[0]);
        for (int k = 1; k <= changedTiles[0]; k++) {
            printf("%d, ", changedTiles[k]);
        }
        printf("\n");

        for (int k = 1; k <= changedTiles[0]; k++) {
            tiHeapRefresh(self->context, self->bset, changedTiles[k]);
        }
        tiRefreshValues(self->context, self->bset, tID);
        return 1;
    } else {
        printf(" Ripple changes failed!\n");
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

#define xtil(tID) (tID % context->xSize)
#define ytil(tID) (tID / context->xSize)

#define xTileID(tID) (tID % context->xSize)
#define yTileID(tID) (tID / context->xSize)

static int processChildTile(Engine self, unsigned int tID, cardinal dir, Bitmask cDifference, unsigned int * changedTiles, VisitNode * toVisit) {
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

    printf("      - In ChildTile (%d, %d) / p rmv:", xTileID(ctID), yTileID(ctID));
    bmPrint(cDifference);
    printf("\n");

    tile * cTile = &(context->tiles[ctID]);
    unsigned int diffBlockIDs[bsetLen(bset) + 1];

    cTile = &(context->tiles[ctID]);
    printf("        - Adj sTile (%d, %d) with vbm:", xtil(ctID), ytil(ctID));
    bmPrint(cTile->validBlockMask);
    printf("\n");

    bmAnd(cDifference, cTile->validBlockMask);
    bmFastCherrypick(cDifference, diffBlockIDs);
    unsigned int index = diffBlockIDs[0];

    printf("        - After cherrypick cDiff is:");
    bmPrint(cDifference);
    printf("\n");

    while (index > 0) {
        printf("          - Would add(?) block %d\n", diffBlockIDs[index]);
        Block curBlock = bsetLookup(bset, diffBlockIDs[index--]);

        if (! bmAndValue(curBlock->overlapMasks[dir], curTile->validBlockMask ) ) {
            blbmAdd(curBlock, cDifference);
            printf("          - Actually adding block %d, leaves cDiff as:", diffBlockIDs[index+1]);
            bmPrint(cDifference);
            printf("\n");
        }
    }

    if (bmTrue(cDifference)) {
        VisitNode nextToVisit = malloc(sizeof(struct visitNode));
        nextToVisit->tID = ctID;
        nextToVisit->next = *toVisit;
        *toVisit = nextToVisit;

        changedTiles[++changedTiles[0]] = ctID;

        bmOr(cTile->rippleDifference, cDifference);
        bmNot(cDifference);
        bmAnd(cTile->validBlockMask, cDifference);
    }
    return 1;
}

static int rippleChangesFrom(Engine self, unsigned int tID, unsigned int * changedTiles) {
    printf("   In call to rippleChangesFrom()!\n");
    changedTiles[0] = 0;

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

        printf("   - Visiting tile (%d, %d) with vbm:", xTileID(tID), yTileID(tID));
        bmPrint(curTile->validBlockMask);
        printf("  (falisty %d)\n", bmFalse(curTile->validBlockMask));
        printf("   - Tile also has rippleDiff:");
        bmPrint(curDifference);
        printf("\n");
        if (bmFalse(curTile->validBlockMask)) {
            return 0;
        } else {
            Bitmask nDifference = bmCreate(curDifference->len);
            Bitmask sDifference = bmCreate(curDifference->len);
            Bitmask eDifference = bmCreate(curDifference->len);
            Bitmask wDifference = bmCreate(curDifference->len);

            bmFastCherrypick(curDifference, diffBlockIDs);
            unsigned int index = diffBlockIDs[0];
            while (index > 0) {
                Block curBlock = bsetLookup(bset, diffBlockIDs[index--]);

                bmOr(nDifference, curBlock->overlapMasks[CARD_N]);
                bmOr(sDifference, curBlock->overlapMasks[CARD_S]);
                bmOr(eDifference, curBlock->overlapMasks[CARD_E]);
                bmOr(wDifference, curBlock->overlapMasks[CARD_W]);
            }

            processChildTile(self, tID, CARD_N, nDifference, changedTiles, &toVisit);
            processChildTile(self, tID, CARD_S, sDifference, changedTiles, &toVisit);
            processChildTile(self, tID, CARD_E, eDifference, changedTiles, &toVisit);
            processChildTile(self, tID, CARD_W, wDifference, changedTiles, &toVisit);

            bmDestroy(nDifference);
            bmDestroy(sDifference);
            bmDestroy(eDifference);
            bmDestroy(wDifference);
        }
    }
    printf("   Leaving call to rippleChangesFrom()!\n");
    return 1;
}
