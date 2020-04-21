typedef struct tile tile;
struct tile {
    unsigned char value;

    unsigned int freq;
    float entropy;

    unsigned int heapIndex;
    unsigned int ctIndex;

    Bitmask validBlockMask;
    Bitmask rippleDifference;
};

typedef struct context * Context;
struct context {
    unsigned short xSize;
    unsigned short ySize;
    tile * tiles;
    unsigned int * eHeap;
    unsigned int toCollapse;

    int prepared;
};

Context coCreate(unsigned short xSize, unsigned short ySize);
Context coCopy(Context self);
void coDestroy(Context self);
int coPrepare(Context self, BlockSet bset);
void coPrint(Context self);

void coHeapPrint(Context self);
void coHeapPush(Context self, unsigned int tID);
unsigned int coHeapPop(Context self);
void coHeapRemove(Context self, unsigned int tID);

void tiCollapseTo(Context self, unsigned int tID, Block block);
void tiRefreshValues(Context self, BlockSet bset, unsigned int tID);
void tiHeapRefresh(Context self, BlockSet bset, unsigned int tID);
