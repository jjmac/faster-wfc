typedef struct blockSet * BlockSet;

BlockSet bsetCreate(unsigned char size);
void bsetDestroy(BlockSet self);
void bsetPrint(BlockSet self);

void bsetAppend(BlockSet self, Block block);

void bsetLock(BlockSet self);

Block bsetLookup(BlockSet self, unsigned int blockID);
Block bsetRandom(BlockSet self, Bitmask mask, int roll);
void bsetEntropy(BlockSet bset, Bitmask bm, unsigned int * freq, float * entropy);

Bitmask bsetTrueMask(BlockSet self);
Bitmask bsetFalseMask(BlockSet self);
