#ifndef TYPES_H_
#define TYPES_H_

typedef uint64_t chunk;
#define BITSPERCHUNK 64
#define ZERO 0ULL
#define ONE 1ULL
typedef uint32_t agent;
typedef uint32_t edge;
typedef uint32_t id;
typedef float value;
typedef struct { id i; agent c[K + 1]; value v; UT_hash_handle hh; } var;

#endif  /* TYPES_H_ */
