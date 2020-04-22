typedef struct blockSet * BlockSet;

BlockSet bsetCreate(unsigned char size);
BlockSet bsetCreateFromGrid(Grid grid, unsigned char size, int rotations, int reflections);
void bsetDestroy(BlockSet self);
void bsetPrint(BlockSet self);

void bsetAppend(BlockSet self, Block block);
void bsetAppendFromString(BlockSet self, char * str);
void bsetAppendFromGrid(BlockSet self, Grid grid, int rotations, int reflections);

void bsetLock(BlockSet self);

unsigned int bsetLen(BlockSet self);
Block bsetLookup(BlockSet self, unsigned int blockID);
Block bsetRandom(BlockSet self, Bitmask mask, int roll);
void bsetEntropy(BlockSet bset, Bitmask bm, unsigned int * freq, float * entropy);
char bsetBlockToValue(BlockSet self, Bitmask bm);

Bitmask bsetTrueMask(BlockSet self);
Bitmask bsetFalseMask(BlockSet self);
Bitmask bsetPlacableMask(BlockSet self, int tileDirs);

Bitmask bsetbmCreate(BlockSet self);

Bitmask bsetInverseValueMask(BlockSet self, char value);

int bsetTestSymmetry(BlockSet self);
