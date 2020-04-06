#include "cardinals.h"
#include "grids.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"
#include <stdio.h>

BlockSet bsDefault1();


int main(int argc, char** argv) {

    BlockSet bset = bsDefault1();
    return 0;

//    BlockSet bset = bsetCreate( 2 );

//    bsetAppend(bset, blCreateFromString(2, "aaaa"));

//    bsetAppend(bset, blCreateFromString(2, "..aa"));
//    bsetAppend(bset, blCreateFromString(2, "aa.."));
/*
    bsetAppend(bset, blCreateFromString(2, "aaaa"));
    bsetAppend(bset, blCreateFromString(2, "bbbb"));
    bsetAppend(bset, blCreateFromString(2, "cccc"));
    bsetAppend(bset, blCreateFromString(2, "dddd"));
    bsetAppend(bset, blCreateFromString(2, "dddd"));
*/

    bsetLock(bset);

    Context con = coCreate( 20, 20 );
    Engine en = enCreate(bset, con, 0);

    enRun(en);

    return 0;
}

BlockSet bsDefault1() {
    char * preset = "" \
    "...." \
    "##.." \
    "..#." \
    "..#.";

    Grid grid = grCreateFromString(preset, 4, 4);
    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 3);
    bsetPrint(bset);
    return bset;
}
