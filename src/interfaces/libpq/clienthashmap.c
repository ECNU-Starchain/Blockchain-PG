#include <stdlib.h>
#include <string.h>
#include "clienthashmap.h"



static Pair *pair_new(int64_t key, void *value);
static Pair **get_bucket(Hashmap *hashmap, int64_t key);
static Pair **get_pair_ptr(Hashmap *hashmap, int64_t key);
static void init_buckets(Hashmap *hashmap);
static void free_buckets(Hashmap *hashmap);


/* The keys need to be hashed with a self-supplied function. */
 size_t myhash(int64_t key) {
    size_t hash = 0;
    size_t i = 0;
    for (i = 0; i < sizeof(key); i++) {
        hash = key % 10000000;
    }

    return hash;
}

/* Comparison functions must return true if the keys are equal,
 * false otherwise.
 */
 bool compare(int64_t key1, int64_t key2) {
    return key1 - key2 == 0;
}


Hashmap *hashmap_new(size_t num_buckets,
                     HashFunction hash, ComparisonFunction compare) {
    Hashmap *hashmap = (Hashmap*)malloc(sizeof(Hashmap));
    if (hashmap == NULL) {
        goto error_hashmap_alloc;
    }

    hashmap->buckets = (Pair **)malloc(num_buckets * sizeof(Pair *));
    if (hashmap->buckets == NULL) {
        goto error_buckets_alloc;
    }

    hashmap->num_buckets = num_buckets;
    hashmap->hash = hash;
    hashmap->compare = compare;
    hashmap->size = 0;

    init_buckets(hashmap);

    return hashmap;

error_buckets_alloc:
    free(hashmap);
error_hashmap_alloc:
    return NULL;
}

void hashmap_free(Hashmap *hashmap) {
    free_buckets(hashmap);
    free(hashmap->buckets);
    free(hashmap);
}

int hashmap_set(Hashmap *hashmap, int64_t key,
                void *value) {
    Pair **pair_ptr = get_pair_ptr(hashmap, key);

    if (*pair_ptr == NULL) {
        *pair_ptr = pair_new(key, value);
        hashmap->size++;
    } else {
        (*pair_ptr)->value = value;
    }

    return *pair_ptr != NULL;
}

void *hashmap_get(Hashmap *hashmap, int64_t key) {
    Pair *pair = *get_pair_ptr(hashmap, key);
    return pair != NULL ? pair->value : NULL;
}

void *hashmap_delete(Hashmap *hashmap, int64_t  key) {
    void *value = NULL;
    Pair **pair_ptr = get_pair_ptr(hashmap, key);
    if (*pair_ptr != NULL) {
        value = (*pair_ptr)->value;
        Pair *next_pair = (*pair_ptr)->next;
        free(*pair_ptr);
        *pair_ptr = next_pair;
    }

    return value;
}

static Pair *pair_new(int64_t key, void *value) {
    /* Since key is const, it'll have to be copied over from a struct that's
     * already initialized.
     */
    Pair init = {.key = key, .value = value, .next = NULL};
    Pair *pair = (Pair*)malloc(sizeof(Pair));
    if (pair != NULL) {
        memcpy(pair, &init, sizeof(Pair));
    }

    return pair;
}

static Pair **get_bucket(Hashmap *hashmap, int64_t key) {
    return hashmap->buckets + hashmap->hash(key) % hashmap->num_buckets;
}

static Pair **get_pair_ptr(Hashmap *hashmap, int64_t key) {
    Pair **pair_ptr = get_bucket(hashmap, key);
    while (*pair_ptr != NULL) {
        if (hashmap->compare((*pair_ptr)->key, key)) {
            break;
        }
        pair_ptr = &(*pair_ptr)->next;
    }

    return pair_ptr;
}

static void init_buckets(Hashmap *hashmap) {
    /* Initializing the buckets to NULL makes it easy to check if a bucket
     * is already in use.
     */
    size_t i = 0;
    for ( i= 0; i < hashmap->num_buckets; i++) {
        hashmap->buckets[i] = NULL;
    }
}

static void free_buckets(Hashmap *hashmap) {
    size_t i = 0;
    for (i = 0; i < hashmap->num_buckets; i++) {
        Pair *pair = hashmap->buckets[i];
        Pair *prev_pair = NULL;
        while (pair != NULL) {
            prev_pair = pair;
            pair = pair->next;
            free(prev_pair);
        }
    }
}
