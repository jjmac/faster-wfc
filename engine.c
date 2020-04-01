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
            nTile = &(context->tiles[ tID + 1 ]);

            while (int k = bmCherrypick(curDifference)) {
                
            }

        }
    }
    return 1;
}
