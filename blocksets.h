typedef struct blockSet * BlockSet;

BlockSet bsetCreate(unsigned char size);
void bsetDestroy(BlockSet self);
void bsetPrint(BlockSet self);

void bsetAppend(BlockSet self, Block block);

void bsetLock(BlockSet self);

Block bsetLookup(BlockSet self, unsigned int blockID);
Block bsetRandom(BlockSet self, Bitmask mask, int rseed);
float bsetEntropy(BlockSet self, Bitmask mask);

Bitmask bsetTrueMask(BlockSet self);
