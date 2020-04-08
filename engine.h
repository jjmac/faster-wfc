typedef struct engine * Engine;

Engine enCreate(BlockSet bset, int xSize, int ySize);
void enDestroy(Engine self);
int enRun(Engine self, int rSeed);

int enCoerceTile(Engine self, unsigned int tID, char value);
int enCoerceXY(Engine self, unsigned int x, unsigned int y, char value);

void enPrepare(Engine self, int rSeed);
int enRecursiveCoreLoop(Engine self, int maxContradictions, int checkpointInterval);
void enCleanup(Engine self);
void enPrint(Engine self);
