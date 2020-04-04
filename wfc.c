#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"
#include <stdio.h>

int main(int argc, char** argv) {

    BlockSet bset = bsetCreate( 2 );

//    bsetAppend(bset, blCreateFromString(2, "aa.."));
//    bsetAppend(bset, blCreateFromString(2, "..aa"));
    bsetAppend(bset, blCreateFromString(2, "aaaa"));
    bsetAppend(bset, blCreateFromString(2, "bbbb"));
    bsetAppend(bset, blCreateFromString(2, "cccc"));
    bsetAppend(bset, blCreateFromString(2, "dddd"));

    bsetLock(bset);

    Context con = coCreate( 2,2 );
    Engine en = enCreate(bset, con, 0);

    enRun(en);

    return 0;
}
