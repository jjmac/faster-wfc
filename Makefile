wfc: bitmasks.c blocks.c blocksets.c tiles.c engine.c grids.c bitmasks.h blocks.h blocksets.h tiles.h engine.h grids.h wfc.c
	gcc -Wall -o wfc bitmasks.c blocks.c blocksets.c tiles.c engine.c grids.c wfc.c -lm -O3
