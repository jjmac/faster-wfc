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
BlockSet redMaze();
BlockSet flowers(int size);
BlockSet corid(int size);

int main(int argc, char** argv) {

    BlockSet bset = flowers(3);
//    BlockSet bset = bsDefault3();

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

//    bsetPrint(bset);

    Context con = coCreate( 10, 10 );
    Engine en = enCreate(bset, con, 0);

//    enPrepare(en);
    enRun(en);

    return 0;
}

BlockSet bsDefault1() {
    char * preset = "" \
    ".a" \
    "##";

    Grid grid = grCreateFromString(preset, 2, 2);
//    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 2, 0,0);
//    bsetPrint(bset);
    return bset;
}

BlockSet bsDefault2() {
    char * preset = "" \
    "AAA" \
    "..." \
    "xxx";

    Grid grid = grCreateFromString(preset, 3, 3);
//    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 3, 0,0);
//    bsetPrint(bset);
    return bset;
}

BlockSet bsDefault3() {
    char * preset = "" \
    "bbbb" \
    "bbbb" \
    "aaaa" \
    "aaaa";

    Grid grid = grCreateFromString(preset, 4, 4);
//    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, 4, 0,0);
//    bsetPrint(bset);
    return bset;
}

BlockSet redMaze() {
    char * preset = "" \
    "...." \
    ".###" \
    ".#0#" \
    ".###";

    Grid grid = grCreateFromString(preset, 4, 4);
    BlockSet bset = bsetCreateFromGrid(grid, 2, 1,1);
    return bset;
}


BlockSet corid(int size) {
    char * preset = "" \
    "#...#....#c#.rrrr." \
    "##########c#.rrrr." \
    "#cccccccc#c#.rrrr." \
    "##########c#.rrrr." \
    "#....#...#c#......" \
    "#.rr.#.r.#c#######" \
    "#.rr.#...#c#......" \
    "#.rr.#######.rrrr." \
    "#.rr.#.....#......" \
    "#.rr.#.rrr.#######" \
    "#....#.....#......" \
    "############.rrrr." \
    "#...#....#c#.rrrr." \
    "#.r.#.rr.#c#.rrrr." \
    "#.r.#.rr.#c#.rrrr." \
    "#.r.#.rr.#c#.rrrr.";

    Grid grid = grCreateFromString(preset, 18, 16);
    BlockSet bset = bsetCreateFromGrid(grid, size, 1,1);
    return bset;
}

BlockSet flowers(int size) {
    char * preset = "" \
    "..............." \
    ".....o.......o." \
    "....o+o.....o+o" \
    ".....o.......o." \
    ".....+.......+." \
    ".....++.....++." \
    ".o....++...++.." \
    "o+o....+...+..." \
    ".o....++...++.." \
    ".+....+..o..++." \
    ".++..++.o+o..+." \
    "..+.++...o...+." \
    "..+++....+..++." \
    "...+.....+.++.." \
    "...++....+++..." \
    "....++....+...." \
    ".....++..++...." \
    "......+.++....." \
    "......+++......" \
    ".......+......." \
    ".......+......." \
    "ggggggg+ggggggg" \
    "ggggggggggggggg";

    Grid grid = grCreateFromString(preset, 15, 23);
//    grPrint(grid);

    BlockSet bset = bsetCreateFromGrid(grid, size, 1,1);
//    bsetPrint(bset);
    return bset;
}
