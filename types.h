#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
typedef uint64_t chunk;
#define BITSPERCHUNK 64
#define ZERO 0ULL
#define ONE 1ULL
typedef uint32_t id;
typedef float value;
typedef struct { id x, y; } idc;

#endif  /* TYPES_H_ */
