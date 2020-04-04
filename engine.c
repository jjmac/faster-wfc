#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"

struct engine {
    BlockSet blockSet;
    Context context;
    int rSeed;
};

static int advance(Engine self);
static int rippleChangesFrom(Engine self, unsigned int tID, unsigned int * changedTiles);

Engine enCreate(BlockSet blockSet, Context context, int rSeed) {
    Engine self = malloc(sizeof(struct engine));
    self->blockSet = blockSet;
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
    BlockSet blockSet = self->blockSet;

    printf("= In prepare!\n");

    for (unsigned int tID = context->xSize*context->ySize - 1; tID != -1; tID--) {
        context->tiles[tID].validBlockMask = bsetTrueMask(blockSet);
        context->tiles[tID].rippleDifference = bsetFalseMask(blockSet);
        tiRefreshValues(context, blockSet, tID);

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
    BlockSet blockSet = self->blockSet;

    for (unsigned int tID = 0; tID < context->xSize*context->ySize; tID++) {
        context->tiles[tID].value = bsetBlockToValue(blockSet, context->tiles[tID].validBlockMask);
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
    Block block = bsetRandom(self->blockSet, self->context->tiles[tID].validBlockMask, rand() % (self->context->tiles[tID].freq));

    printf(" Got random blockPtr %p\n", block);
    printf(" Collapsing tile %d to block:\n", tID);
    blPrint(block);

    tiCollapseTo(self->context, tID, block);
    printf(" Done tile collapse!\n");

    unsigned int * changedTiles = malloc(sizeof(unsigned int) * 100);
    if (rippleChangesFrom(self, tID, changedTiles)) {
        printf(" Finished rippleChangesFrom() - now refreshing freq/entropy values!\n");
        printf(" Got %d changed tiles: ", changedTiles[0]);
        for (int k = 1; k <= changedTiles[0]; k++) {
            printf("%d, ", changedTiles[k]);
        }
        printf("\n");

        for (int k = 1; k <= changedTiles[0]; k++) {
            tiRefreshValues(self->context, self->blockSet, changedTiles[k]);
        }

        tiRefreshValues(self->context, self->blockSet, tID);
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

static int rippleChangesFrom(Engine self, unsigned int tID, unsigned int * changedTiles) {
    printf("   In call to rippleChangesFrom()!\n");
    changedTiles[0] = 0;

    Context context = self->context;
    BlockSet blockSet = self->blockSet;
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

        printf("   - Visiting tile (%d, %d) with vbm:", xtil(tID), ytil(tID));
        bmPrint(curTile->validBlockMask);
        printf("  (falisty %d)\n", bmFalse(curTile->validBlockMask));
        printf("   - Tile also has rippleDiff:");
        bmPrint(curDifference);
        printf("\n");
        if (bmFalse(curTile->validBlockMask)) {
            return 0;
        } else {
            int nID = tID - context->xSize;
            int sID = tID + context->xSize;
            int eID = tID + 1;
            int wID = tID - 1;

            tile * nTile;
            tile * sTile;
            tile * eTile;
            tile * wTile;

            Bitmask nDifference = bmCreate(curDifference->len);
            Bitmask sDifference = bmCreate(curDifference->len);
            Bitmask eDifference = bmCreate(curDifference->len);
            Bitmask wDifference = bmCreate(curDifference->len);

            bmFastCherrypick(curDifference, diffBlockIDs);
            unsigned int index = diffBlockIDs[0];
            while (index > 0) {
                Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);

                bmOr(nDifference, curBlock->overlapMasks[CARD_N]);
                bmOr(sDifference, curBlock->overlapMasks[CARD_S]);
                bmOr(eDifference, curBlock->overlapMasks[CARD_E]);
                bmOr(wDifference, curBlock->overlapMasks[CARD_W]);
            }

            if (nID >= 0 && nID < context->xSize * context->ySize) {
                nTile = &(context->tiles[ nID ]);
                printf("     - Adj nTile (%d, %d) with vbm:", xtil(nID), ytil(nID));
                bmPrint(nTile->validBlockMask);
                printf("\n");
            } else {
                nTile = NULL;
            }
            if (nTile != NULL) {
                printf("      - In nTile (%d, %d) / pot removing:", xtil(nID), ytil(nID));
                bmPrint(nDifference);
                printf("\n");

                bmAnd(nDifference, nTile->validBlockMask);
                bmFastCherrypick(nDifference, diffBlockIDs);
                index = diffBlockIDs[0];
                while (index > 0) {
                    Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);
                    if ( ! bmAndValue(curBlock->overlapMasks[CARD_N], curTile->validBlockMask ) ) {
                        blbmAdd(curBlock, nDifference);
                    }
                }

                printf("       - After winnow, actual removing:");
                bmPrint(nDifference);
                printf("\n");

                if (bmTrue(nDifference)) {
                    VisitNode nextToVisit = malloc(sizeof(struct visitNode));
                    nextToVisit->tID = nID;
                    nextToVisit->next = toVisit;
                    toVisit = nextToVisit;

                    changedTiles[++changedTiles[0]] = nID;

                    bmOr(nTile->rippleDifference, nDifference);
                    bmNot(nDifference);
                    bmAnd(nTile->validBlockMask, nDifference);

                    printf("       - Adding nTile - tile w/ tID %d pos (%d, %d)\n", nID, xtil(nID), ytil(nID));
//                    printf("       - Adding nTile - tile w/ tID %d pos (%d, %d)\n", nTile->tID, xtil(nTile->tID), ytil(nTile->tID));
                }
            }
/*
                if wDisabledBlocks:
                    rprint ("  removing w", wDisabledBlocks, "wTile was", wTile.validBlockMask)
                    toVisit.add(wTile)
                    changedTiles.add(wTile)
                    wTile.preReduce(wDisabledBlocks)
                    rprint ("    - wTile now", wTile.validBlockMask)
*/

            if (sID >= 0 && sID < context->xSize * context->ySize) {
                sTile = &(context->tiles[ sID ]);
                printf("     - Adj sTile (%d, %d) with vbm:", xtil(sID), ytil(sID));
                bmPrint(sTile->validBlockMask);
                printf("\n");
            } else {
                sTile = NULL;
            }
            if (sTile != NULL) {
                bmAnd(sDifference, sTile->validBlockMask);
                bmFastCherrypick(sDifference, diffBlockIDs);
                index = diffBlockIDs[0];
                while (index > 0) {
                    Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);
                    if ( ! bmAndValue(curBlock->overlapMasks[CARD_S], curTile->validBlockMask ) ) {
                        blbmAdd(curBlock, sDifference);
                    }
                }

                if (bmTrue(sDifference)) {
                    VisitNode nextToVisit = malloc(sizeof(struct visitNode));
                    nextToVisit->tID = sID;
                    nextToVisit->next = toVisit;
                    toVisit = nextToVisit;

                    changedTiles[++changedTiles[0]] = sID;

                    bmOr(sTile->rippleDifference, sDifference);
                    bmNot(sDifference);
                    bmAnd(sTile->validBlockMask, sDifference);
                }
            }

            if (eID >= 0 && eID < context->xSize * context->ySize) {
                eTile = &(context->tiles[ eID ]);
                printf("     - Adj eTile (%d, %d) with vbm:", xtil(eID), ytil(eID));
                bmPrint(eTile->validBlockMask);
                printf("\n");
            } else {
                eTile = NULL;
            }
            if (eTile != NULL) {
                bmAnd(eDifference, eTile->validBlockMask);
                bmFastCherrypick(eDifference, diffBlockIDs);
                index = diffBlockIDs[0];
                while (index > 0) {
                    Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);
                    if ( ! bmAndValue(curBlock->overlapMasks[CARD_E], curTile->validBlockMask ) ) {
                        blbmAdd(curBlock, eDifference);
                    }
                }

                if (bmTrue(eDifference)) {
                    VisitNode nextToVisit = malloc(sizeof(struct visitNode));
                    nextToVisit->tID = eID;
                    nextToVisit->next = toVisit;
                    toVisit = nextToVisit;

                    changedTiles[++changedTiles[0]] = eID;

                    bmOr(eTile->rippleDifference, eDifference);
                    bmNot(eDifference);
                    bmAnd(eTile->validBlockMask, eDifference);
                }
            }

            if (wID >= 0 && wID < context->xSize * context->ySize) {
                wTile = &(context->tiles[ wID ]);
                printf("     - Adj wTile (%d, %d) with vbm:", xtil(wID), ytil(wID));
                bmPrint(wTile->validBlockMask);
                printf("\n");
            } else {
                wTile = NULL;
            }
            if (wTile != NULL) {
                bmAnd(wDifference, wTile->validBlockMask);
                bmFastCherrypick(wDifference, diffBlockIDs);
                index = diffBlockIDs[0];
                while (index > 0) {
                    Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);
                    if ( ! bmAndValue(curBlock->overlapMasks[CARD_W], curTile->validBlockMask ) ) {
                        blbmAdd(curBlock, wDifference);
                    }
                }

                if (bmTrue(wDifference)) {
                    VisitNode nextToVisit = malloc(sizeof(struct visitNode));
                    nextToVisit->tID = wID;
                    nextToVisit->next = toVisit;
                    toVisit = nextToVisit;

                    changedTiles[++changedTiles[0]] = wID;

                    bmOr(wTile->rippleDifference, wDifference);
                    bmNot(wDifference);
                    bmAnd(wTile->validBlockMask, wDifference);
                }
            }


        }
    }
    printf("   Leaving call to rippleChangesFrom()!\n");
    return 1;
}
