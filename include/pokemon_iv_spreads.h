const u8 WORST_IV_SPREAD[NUMBER_OF_MON_TYPES][NUM_STATS] =
{
    [TYPE_NORMAL]   = {0, 0, 0, 0, 0, 0},
    [TYPE_FIGHTING] = {1, 1, 0, 0, 0, 0},
    [TYPE_FLYING]   = {1, 1, 1, 0, 0, 0},
    [TYPE_POISON]   = {1, 1, 0, 1, 0, 0},
    [TYPE_GROUND]   = {1, 1, 1, 1, 0, 0},
    [TYPE_ROCK]     = {1, 1, 0, 0, 1, 0},
    [TYPE_BUG]      = {1, 1, 1, 0, 1, 0},
    [TYPE_GHOST]    = {1, 0, 1, 1, 1, 0},
    [TYPE_STEEL]    = {1, 1, 1, 1, 1, 0},
    [TYPE_FIRE]     = {1, 0, 1, 0, 0, 1},
    [TYPE_WATER]    = {1, 1, 1, 0, 0, 1},
    [TYPE_GRASS]    = {0, 1, 1, 1, 0, 1},
    [TYPE_ELECTRIC] = {1, 1, 1, 1, 0, 1},
    [TYPE_PSYCHIC]  = {0, 1, 1, 0, 1, 1},
    [TYPE_ICE]      = {1, 1, 1, 0, 1, 1},
    [TYPE_DRAGON]   = {0, 1, 1, 1, 1, 1},
    [TYPE_DARK]     = {1, 1, 1, 1, 1, 1}
};

const u8 PASSABLE_IV_SPREAD[NUMBER_OF_MON_TYPES][NUM_STATS] =
{
    [TYPE_NORMAL]   = {15, 15, 15, 15, 15, 15},
    [TYPE_FIGHTING] = {15, 15, 16, 16, 16, 16},
    [TYPE_FLYING]   = {15, 15, 15, 16, 16, 16},
    [TYPE_POISON]   = {15, 15, 16, 15, 16, 16},
    [TYPE_GROUND]   = {15, 15, 15, 15, 16, 16},
    [TYPE_ROCK]     = {15, 15, 16, 16, 15, 16},
    [TYPE_BUG]      = {15, 15, 15, 16, 15, 16},
    [TYPE_GHOST]    = {15, 16, 15, 15, 15, 16},
    [TYPE_STEEL]    = {15, 15, 15, 15, 15, 16},
    [TYPE_FIRE]     = {15, 16, 15, 16, 16, 15},
    [TYPE_WATER]    = {15, 15, 15, 16, 16, 15},
    [TYPE_GRASS]    = {16, 15, 15, 15, 16, 15},
    [TYPE_ELECTRIC] = {15, 15, 15, 15, 16, 15},
    [TYPE_PSYCHIC]  = {16, 15, 15, 16, 15, 15},
    [TYPE_ICE]      = {15, 15, 15, 16, 15, 15},
    [TYPE_DRAGON]   = {16, 15, 15, 15, 15, 15},
    [TYPE_DARK]     = {15, 15, 15, 15, 15, 15}
};

const u8 BEST_IV_SPREAD[NUMBER_OF_MON_TYPES][NUM_STATS] =
{
    [TYPE_NORMAL]   = {31, 31, 31, 31, 31, 31},
    [TYPE_FIGHTING] = {31, 31, 30, 30, 30, 30},
    [TYPE_FLYING]   = {31, 31, 31, 30, 30, 30},
    [TYPE_POISON]   = {31, 31, 30, 31, 30, 30},
    [TYPE_GROUND]   = {31, 31, 31, 31, 30, 30},
    [TYPE_ROCK]     = {31, 31, 30, 30, 31, 30},
    [TYPE_BUG]      = {31, 31, 31, 30, 31, 30},
    [TYPE_GHOST]    = {31, 30, 31, 31, 31, 30},
    [TYPE_STEEL]    = {31, 31, 31, 31, 31, 30},
    [TYPE_FIRE]     = {31, 30, 31, 30, 30, 31},
    [TYPE_WATER]    = {31, 31, 31, 30, 30, 31},
    [TYPE_GRASS]    = {30, 31, 31, 31, 30, 31},
    [TYPE_ELECTRIC] = {31, 31, 31, 31, 30, 31},
    [TYPE_PSYCHIC]  = {30, 31, 31, 30, 31, 31},
    [TYPE_ICE]      = {31, 31, 31, 30, 31, 31},
    [TYPE_DRAGON]   = {30, 31, 31, 31, 31, 31},
    [TYPE_DARK]     = {31, 31, 31, 31, 31, 31}
};
