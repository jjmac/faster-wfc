#include "cardinals.h"
#include "bitmasks.h"
#include "blocks.h"
#include "blocksets.h"
#include "engine.h"

int main(int argc, char** argv) {

    BlockSet bset = bsetCreate( 2 );
    Context con = coCreate( 6,6 );
    Engine en = enCreate(bset, con, 0);

    return 0;
}
