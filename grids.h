typedef struct grid * Grid;

struct grid {
    char * values;
    unsigned short xSize;
    unsigned short ySize;
};

Grid grCreate(unsigned short xSize, unsigned short ySize);
Grid grCreateFromString(char* string, unsigned short xSize, unsigned short ySize);
void grDestroy(Grid self);

void grPrint(Grid self);

char grLookup(Grid self, unsigned short x, unsigned short y);
void grPut(Grid self, unsigned short x, unsigned short y, char value);
