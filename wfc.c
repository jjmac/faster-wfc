#include "cardinals.h"
#include "grids.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"
#include <stdio.h>

BlockSet bsDefault1();
BlockSet bsDefault2();
BlockSet bsDefault3();


int main(int argc, char** argv) {

    BlockSet bset = bsDefault2();

//    BlockSet bset = bsetCreate( 3 );

//    bsetAppend(bset, blCreateFromString(3, "aaa...bbb"));
//    bsetAppend(bset, blCreateFromString(3, "...aaa..."));


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

    Context con = coCreate( 3, 3 );
    Engine en = enCreate(bset, con, 0);

//    enPrepare(en);
    enRun(en);

    return 0;
}

BlockSet bsDefault1() {
    char * preset = "" \
    ".." \
    "##";

    Grid grid = grCreateFromString(preset, 2, 2);
    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 2, 1,0);
    bsetPrint(bset);
    return bset;
}

BlockSet bsDefault2() {
    char * preset = "" \
    "A+." \
    "..." \
    "xxx";

    Grid grid = grCreateFromString(preset, 3, 3);
    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 3, 0,0);
    bsetPrint(bset);
    return bset;
}

BlockSet bsDefault3() {
    char * preset = "" \
    "...." \
    "##.." \
    "..#." \
    "..#.";

    Grid grid = grCreateFromString(preset, 4, 4);
    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 3, 1,1);
    bsetPrint(bset);
    return bset;
}
