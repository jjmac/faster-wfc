#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"
#include <stdio.h>

int main(int argc, char** argv) {

    BlockSet bset = bsetCreate( 2 );

    Block a1 = blCreateFromString(2, "aa..");
    Block a2 = blCreateFromString(2, "aa..");

    bsetAppend(bset, a1);
    bsetAppend(bset, a2);

    bsetLock(bset);

    Context con = coCreate( 2,2 );
    Engine en = enCreate(bset, con, 0);

    enRun(en);

    return 0;
}
