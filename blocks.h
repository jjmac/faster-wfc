typedef struct block * Block;

struct block {
    char * values;
    unsigned char size;

    unsigned int freq;

    unsigned char fieldIndex;
    unsigned char bitIndex;
    field localMask;
    Bitmask fullMask;

    Bitmask overlapMasks[4];
};

#define absIndex(block) (block->fieldIndex*FIELD_LEN + block->bitIndex)

Block blCreate(int size);
Block blCreateFromString(int size, char* str);
void blDestroy(Block self);
void blPrint(Block self);
void blPrintData(Block self);

// TODO: replace with macro??
void blbmAdd(Block self, Bitmask bm);
void blbmRemove(Block self, Bitmask bm);
void blbmSet(Block self, Bitmask bm);

void blMarkOverlaps(Block self, Block other, cardinal dir);
