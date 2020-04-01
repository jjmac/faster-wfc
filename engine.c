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

    //collapse it
    Bitmask difference = tiCollapseTo(self->context, tID, block);

    if (rippleChangesFrom(tID)) {

    }
    return 1;

    /*
    def collapseTileTo(self, tile, block):
        dprint ("     - collapsing", tile, "to", block)

        difference = tile.collapseTo(block)
#        oldBlocks = tile.collapseTo(block)
        if not self.rippleChangesFrom(tile, difference):
            dprint ("     - collapse failed - uncollapsing!")
            tile.uncollapse(difference)
            self.context.entropyHeap.push(tile)
            return False
        return True
    */
    return 1;
}

static int rippleChangesFrom(unsigned int tID, Bitmask bitmask) {

}
