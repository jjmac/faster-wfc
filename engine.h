typedef struct engine * Engine;

Engine enCreate(BlockSet bset, int xSize, int ySize, int rSeed);
void enDestroy(Engine self);
int enRun(Engine self);

void enPrepare(Engine self);
int enRecursiveCoreLoop(Engine self, int maxContradictions, int checkpointInterval);
void enCleanup(Engine self);
void enPrint(Engine self);
