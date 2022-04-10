/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */
#ifndef _MBTREE_H
#define _MBTREE_H
//#include <iostream>
#include <stdio.h>
//#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sha256.h"
#include "pgvector.h"
#include "hashmap.h"
#include "storage/itemptr.h"
//using namespace __gnu_cxx;

#define BPLUS_MIN_ORDER     3
#define BPLUS_MAX_ORDER     50
#define BPLUS_MAX_ENTRIES   50
#define BPLUS_MAX_LEVEL     10
#define PAGESIZE 4096
#define DATA_SIZE 32
typedef int64_t key_bt;

/**********************************************************************/


/**********************************************************************/


typedef struct node_list_head {
    struct node_list_head *prev, *next;
}node_list_head;

static inline void list_init(struct node_list_head *link)
{
    link->prev = link;
    link->next = link;
}

static inline void
__list_add(struct node_list_head *link, struct node_list_head *prev, struct node_list_head *next)
{
    link->next = next;
    link->prev = prev;
    next->prev = link;
    prev->next = link;
}

static inline void __list_del(struct node_list_head *prev, struct node_list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_add(struct node_list_head *link, struct node_list_head *prev)
{
    __list_add(link, prev, prev->next);
}

static inline void list_add_tail(struct node_list_head *link, struct node_list_head *head)
{
    __list_add(link, head->prev, head);
}

static inline void list_del(struct node_list_head *link)
{
    __list_del(link->prev, link->next);
    list_init(link);
}

static inline int64_t list_is_first(struct node_list_head *link, struct node_list_head *head)
{
    return link->prev == head;
}

static inline int64_t list_is_last(struct node_list_head *link, struct node_list_head *head)
{
    return link->next == head;
}

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)



typedef struct bplus_node {
    int64_t type;
    int64_t parent_key_idx;
    struct bplus_non_leaf *parent;
    struct node_list_head link;
    int64_t count;
    ItemPointer tids[BPLUS_MAX_ENTRIES];
}bplus_node;


typedef struct bplus_non_leaf {
    int64_t type;
    int64_t parent_key_idx;
    struct bplus_non_leaf *parent;
    struct node_list_head link;
    int64_t children;
    ItemPointer tids[BPLUS_MAX_ENTRIES];
    int64_t key[BPLUS_MAX_ORDER - 1];
    struct bplus_node *sub_ptr[BPLUS_MAX_ORDER];
    char childhash[BPLUS_MAX_ORDER][33];
    char node_hash[33];
    int64_t chindrenoffset[BPLUS_MAX_ORDER];
    int64_t node_id;
    int64_t parent_id;
}bplus_non_leaf;
void bplus_non_leaf_reset(struct bplus_non_leaf* pg_bplus_non_leaf);
void bplus_non_leaf_serialize(struct bplus_non_leaf* pg_bplus_non_leaf, char* buffer);
void bplus_non_leaf_deserialize(struct bplus_non_leaf* pg_bplus_non_leaf, char* buffer);
int64_t bplus_non_leaf_get_serialize_size(struct bplus_non_leaf* pg_bplus_non_leaf);
void bplus_non_leaf_init(struct bplus_non_leaf* pg_bplus_non_leaf);





typedef struct bplus_leaf {
    int64_t type;
    int64_t parent_key_idx;
    struct bplus_non_leaf *parent;
    struct node_list_head link;
    int64_t entries;
    ItemPointer tids[BPLUS_MAX_ENTRIES];
    int64_t key[BPLUS_MAX_ENTRIES];
    char data[BPLUS_MAX_ENTRIES][33];
    char childhash[BPLUS_MAX_ENTRIES][33];
    char node_hash[33];
    int64_t node_id;
    int64_t parent_id;
}bplus_leaf;
void bplus_leaf_serialize(struct bplus_leaf* pg_bplus_leaf, char* buffer);
void bplus_leaf_deserialize(struct bplus_leaf* pg_bplus_leaf, char* buffer);
int64_t bplus_leaf_get_serialize_size(struct bplus_leaf* pg_bplus_leaf);
void bplus_leaf_init(struct bplus_leaf* pg_bplus_leaf);



typedef struct bplus_tree {
    int64_t order;
    int64_t entries;
    int64_t level;
    struct bplus_node *root;
    struct node_list_head list[BPLUS_MAX_LEVEL];
    int64_t fd;
    int64_t offset;
}bplus_tree;

void mybplus_tree_init(struct bplus_tree * pg_bplus_tree);




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
typedef rangevo* pgvo;
void rangevo_init(struct rangevo* pg_rangevo,int hashmap_size);
void vo_serialize(rangevo* vo, char* buffer);
void vo_deserialize(rangevo* vo, char* buffer);


void set_node_id(struct bplus_tree *tree,Hashmap*  hashp[], int64_t temp_node_id[][500]);

void bplus_tree_dump(struct bplus_tree *tree);

char* bplus_tree_get(struct bplus_tree *tree, key_bt key);

int64_t bplus_tree_store(struct bplus_tree *tree);

//int64_t bplus_tree_get_vo(struct bplus_tree *tree, key_bt key, verification_object &vo);

int64_t bplus_tree_put(struct bplus_tree *tree, key_bt key, char* data, ItemPointer tid);

//int64_t bplus_tree_get_range(struct bplus_tree *tree, key_bt key1, key_bt key2,vector<int64_t> *result);

//int64_t bplus_tree_get_range_disk(struct bplus_tree *tree, key_bt key1, key_bt key2,
//                                  vector<int64_t> *result,int64_t root_offset);

int64_t bplus_tree_get_range_disk_vo(struct bplus_tree *tree, key_bt key1, key_bt key2,int64_t root_offset,
                                     struct pgvector *result,rangevo* vo, struct pgvector *keys_result);

char* reconstruct_root(struct Hashmap* hashp[], rangevo vo, char *root);

struct bplus_tree *bplus_tree_init(int64_t order, int64_t entries);

void bplus_tree_deinit(struct bplus_tree *tree);

void hashmap_serialize(Hashmap* hashp[BPLUS_MAX_LEVEL], int64_t temp_node_id[10][500],  char* buffer);
void hashmap_deserialize(Hashmap* hashp[BPLUS_MAX_LEVEL],  char buffer[]);


#endif  /* _BPLUS_TREE_H */
