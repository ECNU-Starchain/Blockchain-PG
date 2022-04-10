#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdlib.h>
#include <stdbool.h>
#define NUM_BUCKETS 16
#define NUM_STUDENTS 4
#define MAX_NAME_LEN 16


typedef size_t (*HashFunction)(int64_t key);
typedef bool (*ComparisonFunction)(int64_t key1, int64_t key2);
typedef struct Hashmap Hashmap;

typedef struct Pair {
    int64_t key;
    void *value;
    struct Pair *next;
} Pair;

typedef struct Hashmap {
    size_t num_buckets;
    Pair **buckets;
    HashFunction hash;
    ComparisonFunction compare;
    int64_t size;
}Hashmap;


size_t myhash(int64_t key);
bool compare(int64_t key1, int64_t key2);

/* Create a new hashmap.
 * Returns NULL if memory allocation fails.
 */
Hashmap *hashmap_new(size_t num_buckets,
                     HashFunction hash, ComparisonFunction compare);

/* Free the memory used by the hashmap.
 * NOTE: This does NOT free the memory used by the keys and values.
 */
void hashmap_free(Hashmap *hashmap);

/* Set the mapping for a given key.
 * Returns non-zero if memory allocation fails.
 * If old_value is not NULL, the old value of the mapping will be stored there.
 */
int hashmap_set(Hashmap *hashmap, int64_t key,
                void *value);

/* Get the value for a given key.
 * Returns the value or NULL if no mapping exists for the key.
 */
void *hashmap_get(Hashmap *hashmap, int64_t key);

/* Delete a mapping for a given key.
 * Returns the value of the deleted mapping or NULL if it doesn't exist.
 */
void *hashmap_delete(Hashmap *hashmap, int64_t key);

#endif
