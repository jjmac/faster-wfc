#include <stdio.h>
#include <stdlib.h>

#include "grids.h"

Grid grCreate(unsigned short xSize, unsigned short ySize) {
    Grid self = malloc(sizeof(struct grid));
    self->values = malloc(sizeof(char) * xSize * ySize);
    self->xSize = xSize;
    self->ySize = ySize;
    return self;
}

Grid grCreateFromString(char* string, unsigned short xSize, unsigned short ySize) {
    Grid self = grCreate(xSize, ySize);
    for (unsigned int x = self->xSize-1; x != -1; x--) {
        for (unsigned int y = self->ySize-1; y != -1; y--) {
            grPut(self, x, y, string[y*self->xSize + x]);
        }
    }
    return self;
}

void grDestroy(Grid self) {
    free(self->values);
    free(self);
}

void grPrint(Grid self) {
    printf("Printing grid!\n");
    for (unsigned int y = 0; y < self->ySize; y++) {
        for (unsigned int x = 0; x < self->xSize; x++) {
            putchar(grLookup(self, x, y));
        }
        putchar('\n');
    }
}

char grLookup(Grid self, unsigned short x, unsigned short y){
    return self->values[y*self->xSize + x];
}
void grPut(Grid self, unsigned short x, unsigned short y, char value) {
    self->values[y * self->xSize + x] = value;
}
