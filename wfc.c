#include "cardinals.h"
#include "grids.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "tiles.h"
#include "engine.h"
#include <stdio.h>

#include "benchmarking.h"

BlockSet bsDefault1();
BlockSet bsDefault2();
BlockSet bsDefault3();
BlockSet redMaze();
BlockSet flowers(int size);
BlockSet corid(int size);

void testUnplacable();

extern float bCoreLoopTime;
extern float bRippleTime;
extern float bHeapPushTime;
extern float bHeapRemoveTime;
extern float bHeapRefreshValuesTime;
extern float bHeapRefreshTime;
extern int bHeapRefreshCalls;
extern float bSiftDownTime;
extern float bEntropyTime;

void benchprint(){
    printf("Core loop time: %f\n", bCoreLoopTime/CLOCKS_PER_SEC);
    printf("RippleChanges() time: %f\n", bRippleTime/CLOCKS_PER_SEC);
    printf("Heap push time: %f\n", bHeapPushTime/CLOCKS_PER_SEC);
    printf("Heap refresh values time: %f\n", bHeapRefreshValuesTime/CLOCKS_PER_SEC);
    printf("Heap refresh time: %f\n", bHeapRefreshTime/CLOCKS_PER_SEC);
    printf("Heap refresh calls: %d\n", bHeapRefreshCalls);
    printf("Heap remove time: %f\n", bHeapRemoveTime/CLOCKS_PER_SEC);
    printf("Heap sift time: %f\n", bSiftDownTime/CLOCKS_PER_SEC);
    printf("Entropy time: %f\n", bEntropyTime/CLOCKS_PER_SEC);
}

void testMemory() {
    BlockSet bset;
    Block bl;
    Context con;
    Engine en;
    char str[40];

    printf("Testing block\n");

    bl = blCreateFromString(3, "aaa...bbb");
    blDestroy(bl);

    printf("Testing empty block set\n");

    bset = bsetCreate(3);
    bsetDestroy(bset);

    printf("Testing 1-element block set\n");

    bset = bsetCreate(3);
    bl = blCreateFromString(3, "aaa...bbb");
    bsetAppend(bset, bl);
    bsetDestroy(bset);

    printf("Testing locked 1-element blockset\n");

    bset = bsetCreate(3);
    bl = blCreateFromString(3, "aaa...bbb");
    bsetAppend(bset, bl);
    bsetLock(bset);
    bsetDestroy(bset);

    printf("Testing locked 100-element blockset\n");

    bset = bsetCreate(3);
    for (int k = 0; k < 100; k++) {
        sprintf(str, "%d.........", k);
        bl = blCreateFromString(3, str);
        bsetAppend(bset, bl);
    }
    bsetLock(bset);
    bsetDestroy(bset);

    printf("Testing context\n");

    con = coCreate(10, 10);
    coDestroy(con);

    printf("Testing prepared context w/ 1-element blockset\n");

    con = coCreate(10, 10);
    bset = bsetCreate(3);
    bl = blCreateFromString(3, "aaa...bbb");
    bsetAppend(bset, bl);
    bsetLock(bset);
    coPrepare(con, bset);
    coDestroy(con);
    bsetDestroy(bset);

    printf("Testing prepared context w/ 100-element blockset\n");

    con = coCreate(10, 10);
    bset = bsetCreate(3);
    for (int k = 0; k < 100; k++) {
        sprintf(str, "%d.........", k);
        bl = blCreateFromString(3, str);
        bsetAppend(bset, bl);
    }
    bsetLock(bset);
    coPrepare(con, bset);
    coDestroy(con);
    bsetDestroy(bset);

    printf("Testing copied context w/ 100-element blockset\n");

    con = coCreate(10, 10);
    bset = bsetCreate(3);
    for (int k = 0; k < 100; k++) {
        sprintf(str, "%d.........", k);
        bl = blCreateFromString(3, str);
        bsetAppend(bset, bl);
    }
    bsetLock(bset);
    coPrepare(con, bset);
    Context con2 = coCopy(con);
    coDestroy(con);
    coDestroy(con2);
    bsetDestroy(bset);

    printf("Testing reuse of blockset between contexts\n");

    bset = bsetCreate(3);
    for (int k = 0; k < 100; k++) {
        sprintf(str, "%d.........", k);
        bl = blCreateFromString(3, str);
        bsetAppend(bset, bl);
    }
    bsetLock(bset);
    con = coCreate(10, 10);
    coPrepare(con, bset);
    coDestroy(con);
    con = coCreate(10, 10);
    coPrepare(con, bset);
    coDestroy(con);
    bsetDestroy(bset);

    printf("Testing engine\n");

    bset = bsetCreate(3);
    for (int k = 0; k < 100; k++) {
        sprintf(str, "%d.........", k);
        bl = blCreateFromString(3, str);
        bsetAppend(bset, bl);
    }
    bsetLock(bset);
    en = enCreate(bset, 10, 10);
    enDestroy(en);
    bsetDestroy(bset);

    printf("Testing enRun (1 block, 1 tile)\n");

    bset = bsetCreate(2);
    bsetAppend(bset, blCreateFromString(2, "aaaa"));
    bsetLock(bset);
    en = enCreate(bset, 1, 1);
    enRun(en, 0);
    enDestroy(en);
    bsetDestroy(bset);

    printf("Testing enRun (one block, 100 tiles)\n");

    bset = bsetCreate(2);
    bsetAppend(bset, blCreateFromString(2, "aaaa"));
    bsetLock(bset);
    en = enCreate(bset, 10, 10);
    enRun(en, 0);
    enDestroy(en);
    bsetDestroy(bset);

    printf("Testing enRun (many blocks, 100 tiles)\n");

    bset = flowers(4);
    bsetLock(bset);
    en = enCreate(bset, 10, 10);
    enRun(en, 0);
    enDestroy(en);
    coDestroy(con);
    bsetDestroy(bset);


    printf("Testing enRun (many blocks, 1600 tiles)\n");

    bset = flowers(4);
    bsetLock(bset);
    en = enCreate(bset, 40, 40);
    enRun(en, 0);
    enDestroy(en);
    coDestroy(con);
    bsetDestroy(bset);

    printf("All tests passed!\n");
}

int main(int argc, char** argv) {
//    testMemory();
//    return 0;

//    testUnplacable();
//    return 0;

    BlockSet bset = flowers(3);
    bsetLock(bset);

    Engine en = enCreate(bset, 30, 30);

    if(enPrepare(en, 0)) {
/*        if (!enCoerceXY(en, 2,0, 'g')) {
            printf("Coercion failure!\n");
        }
        if (!enCoerceXY(en, 5,0, 'g')) {
            printf("Coercion failure!\n");
        }*/

        enIgnoreXY(en, 2, 9);
        enIgnoreXY(en, 3, 9);
        enIgnoreXY(en, 4, 9);
        enIgnoreXY(en, 5, 9);
        enIgnoreXY(en, 6, 9);
        enIgnoreXY(en, 7, 9);
        enIgnoreXY(en, 8, 9);
        enIgnoreXY(en, 9, 9);
        enIgnoreXY(en, 10, 9);
        enIgnoreXY(en, 11, 9);

        if (enRecursiveCoreLoop(en, 10, 30)) {
            enPrint(en);
        }
    }

    enDestroy(en);
    bsetDestroy(bset);

#if DEBUG_BENCH
    benchprint();
#endif
    return 0;
}

void testUnplacable() {
    BlockSet bset = bsetCreate(3);
    bsetAppendFromString(bset, "aaa......");
    bsetAppendFromString(bset, "......aaa");
    bsetAppendFromString(bset, ".........");

    bsetLock(bset);

    bsetPrint(bset);

    Engine en = enCreate(bset, 1, 4);
    if (enPrepare(en, 12)) {
        if (enRecursiveCoreLoop(en, 10, 30)) {
            enPrint(en);
        }
    }
    enDestroy(en);
    bsetDestroy(bset);
}

BlockSet bsDefault1() {
    char * preset = "" \
    ".." \
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

    BlockSet bset = bsetCreateFromGrid(grid, size, 1,1);

    grDestroy(grid);
    return bset;
}
