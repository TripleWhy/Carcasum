#include "random.h"

int DefaultRandom::add = 0;

int * RandomTable::table = 0;
int RandomTable::tableUse = 0;
int RandomTable::add = 0;
#if RANDOM_TABLE_SHARED_OFFSET
int RandomTable::offset = 0;
#endif
