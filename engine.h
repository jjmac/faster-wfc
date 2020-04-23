typedef struct engine * Engine;

Engine enCreate(BlockSet bset, int xSize, int ySize);
void enDestroy(Engine self);
int enRun(Engine self, int rSeed);

int enCoerceTile(Engine self, unsigned int tID, char value);
int enCoerceXY(Engine self, unsigned int x, unsigned int y, char value);

void enIgnoreTile(Engine self, unsigned int tID);
void enIgnoreXY(Engine self, unsigned int x, unsigned int y);

int enPrepare(Engine self, int rSeed);
int enRecursiveCoreLoop(Engine self, int maxContradictions, int checkpointInterval);
void enCleanup(Engine self);
void enPrint(Engine self);
void enWriteToBuffer(Engine self, char * buffer);
