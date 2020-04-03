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
        context->tiles[tID].entropy = bsetEntropy(blockSet, context->tiles[tID].validBlockMask);


        printf("Tile %d has mask:", tID);
        bmPrint(context->tiles[tID].validBlockMask);
        printf("\n");

        coHeappush(context, tID);
    }
}

void enCoreLoop(Engine self) {
    while (self->context->eHeap[0]) {
        printf("= In core loop - advancing!\n");
        advance(self);
    }
}

void enCleanup(Engine self) {

}

static int advance(Engine self) {
    printf("In call to advance()!\n");

    // pick a tile to collapse
    unsigned int tID = coHeappop(self->context);

    printf(" Got random tID: %d\n", tID);

    printf(" Tile blockmask is %p\n", self->context->tiles[tID].validBlockMask);


    // pick a block to collapse it to
    Block block = bsetRandom(self->blockSet, self->context->tiles[tID].validBlockMask, self->rSeed++);

    printf(" Got random blockPtr %p\n", block);
    printf(" Collapsing tile %d to block:\n", tID);
    blPrint(block);

    tiCollapseTo(self->context, tID, block);
    printf(" Done tile collapse!\n");

    while(1){

    }

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
    printf("In call to rippleChangesFrom()!\n");

    Context context = self->context;
    BlockSet blockSet = self->blockSet;
    unsigned int diffBlockIDs[context->tiles[0].validBlockMask->len * FIELD_LEN + 1];

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

            bmFastCherrypick(curDifference, diffBlockIDs);
            unsigned int index = diffBlockIDs[0];
            while (index > 0) {
                Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);

                bmOr(nDifference, curBlock->overlapMasks[CARD_N]);
                bmOr(sDifference, curBlock->overlapMasks[CARD_S]);
                bmOr(eDifference, curBlock->overlapMasks[CARD_E]);
                bmOr(wDifference, curBlock->overlapMasks[CARD_W]);
            }

            if (nTile != NULL) {
                bmAnd(nDifference, nTile->validBlockMask);
                bmFastCherrypick(nDifference, diffBlockIDs);
                index = diffBlockIDs[0];
                while (index > 0) {
                    Block curBlock = bsetLookup(blockSet, diffBlockIDs[index--]);
                    if ( bmAndFalse(curBlock->overlapMasks[CARD_S], curTile->validBlockMask ) ) {
                        blbmAdd(curBlock, nDifference);
                    }
                }
            }

        }
    }
    return 1;
}
