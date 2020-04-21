typedef enum cardinal cardinal;

enum cardinal {
    CARD_N,
    CARD_S,
    CARD_E,
    CARD_W
};

#define cardChar(c) (c&2 ? (c&1 ? 'w' : 'e') : (c&1 ? 's' : 'n') )
#define cardReverse(c) ( c&2 ? (c&1 ? CARD_E : CARD_W) : (c&1 ? CARD_N : CARD_S) )
#define cardBit(c) (1 << c)

#define NO_CARDS 0
#define ALL_CARDS 0b1111
