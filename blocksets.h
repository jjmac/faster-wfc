typedef struct blockSet * BlockSet;

BlockSet bsetCreate(unsigned char size);
void bsetDestroy(BlockSet self);
void bsetPrint(BlockSet self);

void bsetAppend(BlockSet self, Block block);

void bsetLock(BlockSet self);

Block bsetRandom(BlockSet self, Bitmask mask, int rseed);
float bsetEntropy(BlockSet self, Bitmask mask);
