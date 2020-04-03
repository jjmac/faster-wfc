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
static int rippleChangesFrom(Engine self, unsigned int tID);

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
    enPrepare(self);
    enCoreLoop(self);
    enCleanup(self);
}

void enPrepare(Engine self) {
    Context context = self->context;
    BlockSet blockSet = self->blockSet;

    for (unsigned int tID = context->xSize*context->ySize - 1; tID >= 0; tID--) {
        context->tiles[tID].entropy = bsetEntropy(blockSet, context->tiles[tID].validBlockMask);
        coHeappush(context, tID);
    }
}

void enCoreLoop(Engine self) {
    while (self->context->eHeap[0]) {
        advance(self);
    }
}

void enCleanup(Engine self) {

}

static int advance(Engine self) {
    // pick a tile to collapse
    unsigned int tID = coHeappop(self->context);

    // pick a block to collapse it to
    Block block = bsetRandom(self->blockSet, self->context->tiles[tID].validBlockMask, self->rSeed++);

    tiCollapseTo(self->context, tID, block);

    if (rippleChangesFrom(self, tID)) {
        return 1;
    } else {
        return 0;
    }
}

// TODO: Rewrite this function so that it weaves a linkedlist through context->tiles rather than constructing a paralell list!
typedef struct visitNode * VisitNode;
struct visitNode {
    unsigned int tID;
    VisitNode next;
};

static int rippleChangesFrom(Engine self, unsigned int tID) {
    Context context = self->context;
    BlockSet blockSet = self->blockSet;

    tile * nTile = NULL;
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

            if (nID >= 0 && nID < context->xSize * context->ySize) {
                nTile = &(context->tiles[ nID ]);
            } else {
                nTile = NULL;
            }
            if (sID >= 0 && sID < context->xSize * context->ySize) {
                sTile = &(context->tiles[ sID ]);
            } else {
                sTile = NULL;
            }
            if (eID >= 0 && eID < context->xSize * context->ySize) {
                eTile = &(context->tiles[ eID ]);
            } else {
                eTile = NULL;
            }
            if (wID >= 0 && wID < context->xSize * context->ySize) {
                wTile = &(context->tiles[ wID ]);
            } else {
                wTile = NULL;
            }

            Bitmask nDifference = bmCreate(curDifference->len);
            Bitmask sDifference = bmCreate(curDifference->len);
            Bitmask eDifference = bmCreate(curDifference->len);
            Bitmask wDifference = bmCreate(curDifference->len);

            unsigned int * diffValues = bmFastCherrypick(curDifference);
            unsigned int index = diffValues[0];
            while (index > 0) {
                Block block = bsetLookup(blockSet, diffValues[index--]);

                bmOr(nDifference, block->overlapMasks[CARD_N]);
                bmOr(sDifference, block->overlapMasks[CARD_S]);
                bmOr(eDifference, block->overlapMasks[CARD_E]);
                bmOr(wDifference, block->overlapMasks[CARD_W]);
            }

            if (nTile != NULL) {
                unsigned int * nDiffValues = bmFastCherrypick(nDifference);
                index = nDiffValues[0];
                while (index > 0) {

                }
            }

        }
    }
    return 1;
}
