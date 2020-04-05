typedef struct tile tile;
struct tile {
    unsigned char value;

    unsigned int tID;
    unsigned int heapIndex;

    unsigned int freq;
    float entropy;

    Bitmask validBlockMask;
    Bitmask rippleDifference;
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
void coHeapremove(Context self, unsigned int tID);

void tiCollapseTo(Context self, unsigned int tID, Block block);
void tiRefreshValues(Context self, BlockSet bset, unsigned int tID);
void tiHeapRefresh(Context self, BlockSet bset, unsigned int tID);
