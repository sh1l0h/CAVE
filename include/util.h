#ifndef CAVE_UTIL_H
#define CAVE_UTIL_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdalign.h>

#include "ZINC/include/zinc.h"
#include "core/log.h"
#include "core/direction.h"

#define FIXED_UPDATES_PER_SECOND 120
#define FIXED_DELTA_TIME (1.0f / FIXED_UPDATES_PER_SECOND)

#define POS_2_BLOCK(pos) ZINC_VEC3I(floorf((pos)->x),\
                                    floorf((pos)->y),\
                                    floorf((pos)->z))

#define POS_2_CHUNK(pos) ZINC_VEC3I(floorf((pos)->x / CHUNK_SIZE),\
                                    floorf((pos)->y / CHUNK_SIZE),\
                                    floorf((pos)->z / CHUNK_SIZE))

#define BLOCK_2_CHUNK(pos)                                                     \
    ZINC_VEC3I((((pos)->x < 0 ? -CHUNK_SIZE + 1 : 0) + (pos)->x) / CHUNK_SIZE, \
               (((pos)->y < 0 ? -CHUNK_SIZE + 1 : 0) + (pos)->y) / CHUNK_SIZE, \
               (((pos)->z < 0 ? -CHUNK_SIZE + 1 : 0) + (pos)->z) / CHUNK_SIZE)

#define BLOCK_2_OFFSET_IN_CHUNK(pos)                                    \
    ZINC_VEC3I(((pos)->x < 0 ? CHUNK_SIZE : 0) + (pos)->x % CHUNK_SIZE, \
               ((pos)->y < 0 ? CHUNK_SIZE : 0) + (pos)->y % CHUNK_SIZE, \
               ((pos)->z < 0 ? CHUNK_SIZE : 0) + (pos)->z % CHUNK_SIZE)

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define CLAMP(v, min, max) MAX(min, MIN(max, v))
#define SIGN(a) ((a > 0) ? 1 : ((a < 0) ? -1 : 0))

#ifdef __GNUC__
#define container_of(_ptr, _type, _member)   \
    ({const typeof(((_type *) 0)->_member) *_t = (_ptr); \
      (_type *)((char *) _t - offsetof(_type, _member));})
#else
#define container_of(_ptr, _type, _member)   \
    ((_type *)((char *) (_ptr) - offsetof(_type, _member)))
#endif

#define ARRAY_SIZE(_arr) \
    (sizeof((_arr)) / sizeof((_arr)[0]))

#define CACHE_LINE_SIZE 64 // Typical cache line size for modern architectures

#endif
