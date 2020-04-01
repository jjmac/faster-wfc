typedef struct tile tile;
struct tile {
    unsigned char value;

    unsigned int tID;
    unsigned int heapIndex;

    unsigned int frequency;
    float entropy;

    Bitmask validBlockMask;
    Bitmask oldValidBlockMask;
};

typedef struct context * Context;
struct context {
    unsigned short xSize;
    unsigned short ySize;
    tile * tiles;
    unsigned int * eHeap;
};

Context coCreate(unsigned short xSize, unsigned short ySize);
void coDestroy(Context self);
void coPrint(Context self);

void coHeappush(Context self, unsigned int tID);
unsigned int coHeappop(Context self);
void coHeaprefresh(Context self, unsigned int tID);

Bitmask tiCollapseTo(Context self, unsigned int tID, Block block);
