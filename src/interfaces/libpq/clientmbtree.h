/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */
#ifndef _BPLUS_TREE_H
#define _BPLUS_TREE_H
//#include <iostream>
#include <stdio.h>
//#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "clientsha256.h"
#include "clientpgvector.h"
#include "clienthashmap.h"
//using namespace __gnu_cxx;

#define BPLUS_MIN_ORDER     3
#define BPLUS_MAX_ORDER     70
#define BPLUS_MAX_ENTRIES   70
#define BPLUS_MAX_LEVEL     10
#define PAGESIZE 4096
#define DATA_SIZE 32
typedef int64_t key_bt;

/**********************************************************************/
typedef unsigned char uint8;	/* == 8 bits */
typedef unsigned short uint16;	/* == 16 bits */
typedef unsigned int uint32;	/* == 32 bits */
typedef uint16 OffsetNumber;
typedef struct BlockIdData
{
    uint16		bi_hi;
    uint16		bi_lo;
} BlockIdData;
typedef uint16 OffsetNumber;
typedef BlockIdData *BlockId;	/* block identifier */

typedef struct ItemPointerData
{
    BlockIdData ip_blkid;
    OffsetNumber ip_posid;
}ItemPointerData;
typedef ItemPointerData *ItemPointer;

/**********************************************************************/

typedef struct list_head {
    struct list_head *prev, *next;
}list_head;

static inline void list_init(struct list_head *link)
{
    link->prev = link;
    link->next = link;
}

static inline void
__list_add(struct list_head *link, struct list_head *prev, struct list_head *next)
{
    link->next = next;
    link->prev = prev;
    next->prev = link;
    prev->next = link;
}



static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_add(struct list_head *link, struct list_head *prev)
{
    __list_add(link, prev, prev->next);
}

static inline void list_add_tail(struct list_head *link, struct list_head *head)
{
    __list_add(link, head->prev, head);
}

static inline void list_del(struct list_head *link)
{
    __list_del(link->prev, link->next);
    list_init(link);
}

static inline int64_t list_is_first(struct list_head *link, struct list_head *head)
{
    return link->prev == head;
}

static inline int64_t list_is_last(struct list_head *link, struct list_head *head)
{
    return link->next == head;
}

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)
typedef struct bplus_non_leaf {
    int64_t type;
    int64_t parent_key_idx;
    struct bplus_non_leaf *parent;
    struct list_head link;
    int64_t children;
    ItemPointer tids[BPLUS_MAX_ENTRIES];
    int64_t key[BPLUS_MAX_ORDER - 1];
    struct bplus_node *sub_ptr[BPLUS_MAX_ORDER];
    char childhash[BPLUS_MAX_ORDER][125];
    char node_hash[125];
    int64_t chindrenoffset[BPLUS_MAX_ORDER];
    int64_t node_id;
    int64_t parent_id;
}bplus_non_leaf;

typedef struct bplus_node {
    int64_t type;
    int64_t parent_key_idx;
    struct bplus_non_leaf *parent;
    struct list_head link;
    int64_t count;
    ItemPointer tids[BPLUS_MAX_ENTRIES];
}bplus_node;




typedef struct bplus_leaf {
    int64_t type;
    int64_t parent_key_idx;
    struct bplus_non_leaf *parent;
    struct list_head link;
    int64_t entries;
    ItemPointer tids[BPLUS_MAX_ENTRIES];
    int64_t key[BPLUS_MAX_ENTRIES];
    char data[BPLUS_MAX_ENTRIES][125];
    char childhash[BPLUS_MAX_ENTRIES][125];
    char node_hash[125];
    int64_t node_id;
    int64_t parent_id;
}bplus_leaf;

typedef struct bplus_tree {
    int64_t order;
    int64_t entries;
    int64_t level;
    struct bplus_node *root;
    struct list_head list[BPLUS_MAX_LEVEL];
    int64_t fd;
    int64_t offset;
}bplus_tree;

typedef struct siblinghash
{
    int sibling_bumber;
    char sibling[BPLUS_MAX_ENTRIES][1000];
}siblinghash;

typedef struct siblingnode
{
    siblinghash *leftsibling;
    siblinghash *rightsibling;
}siblingnode;

typedef struct verification_object{
    int traverse_height;
    siblingnode node[BPLUS_MAX_LEVEL];
    char leafdata[125];
}verification_object;

enum {
    BPLUS_TREE_LEAF,
    BPLUS_TREE_NON_LEAF = 1,
};

static inline int64_t is_leaf(struct bplus_node *node)
{
    return node->type == BPLUS_TREE_LEAF;
}

typedef struct nodevo
{
    // first node id
    int64_t node_id;
    int64_t bound_node_childno;
//    vector<char*> nhash;
    pgvector nhash;
}nodevo;

typedef struct rangevo
{
    int traverse_height;
    nodevo leftvo[BPLUS_MAX_LEVEL];
    nodevo rightvo[BPLUS_MAX_LEVEL];
    pgvector data;
    pgvector itemdata;
   //by zcy modified 2020/6/30
    char* hashmap;
    int mbtree_size;
    //modified end
//    pgvector result;
//    vector<vector<int64_t>>data;
}rangevo;
void rangevo_init(struct rangevo* pg_rangevo,int hashmap_size);
void vo_serialize(rangevo* vo, char* buffer);
void vo_deserialize(rangevo* vo, char* buffer);

void* rr_leaf_node(void *arg);
char* reconstruct_root(struct Hashmap* hashp[], rangevo vo, char *root);


void hashmap_serialize(Hashmap* hashp[10], int64_t temp_node_id[10][500],  char* buffer);
void hashmap_deserialize(Hashmap* hashp[10],  char buffer[]);



#endif  /* _BPLUS_TREE_H */
