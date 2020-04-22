CC = gcc
CFLAGS = -Wall -c -fPIC -lm -O3
ALLO = bitmasks.o blocks.o blocksets.o tiles.o engine.o grids.o

wfc: ${ALLO} wfc.o
	${CC} -o wfc wfc.o ${ALLO} -lm

clean:
	rm ${ALLO} wfc wfc.so wfc.o

shared: ${ALLO}
	${CC} -o wfc.so -O3 -shared ${ALLO}

wfc.o: wfc.c cardinals.h grids.h bitmasks.h blocks.h blocksets.h tiles.h engine.h benchmarking.h
	${CC} ${CFLAGS} wfc.c

engine.o: engine.c cardinals.h grids.h bitmasks.h blocks.h blocksets.h tiles.h engine.h benchmarking.h
	${CC} ${CFLAGS} engine.c

bitmasks.o: bitmasks.c bitmasks.h
	${CC} ${CFLAGS} bitmasks.c

blocks.o: blocks.c bitmasks.h cardinals.h
	${CC} ${CFLAGS} blocks.c

blocksets.o: blocksets.c blocksets.h blocks.h bitmasks.h grids.h cardinals.h benchmarking.h
	${CC} ${CFLAGS} blocksets.c

grids.o: grids.c grids.h
	${CC} ${CFLAGS} grids.c

tiles.o: tiles.c tiles.h cardinals.h blocksets.h blocks.h grids.h bitmasks.h benchmarking.h
	${CC} ${CFLAGS} tiles.c
