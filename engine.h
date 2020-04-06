typedef struct engine * Engine;

Engine enCreate(BlockSet bset, Context context, int rSeed);
void enDestroy(Engine self);
void enRun(Engine self);

void enPrepare(Engine self);
void enCoreLoop(Engine self);
void enCleanup(Engine self);
