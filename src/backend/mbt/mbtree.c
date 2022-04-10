/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mbt/mbtree.h"
#include <unistd.h>
#define _BPLUS_TREE_DEBUG;
//using namespace __gnu_cxx;
enum {
    LEFT_SIBLING,
    RIGHT_SIBLING = 1,
};
//int count1=0;
//double startTime=0;
//double endTime=0;
//double mytime=0;
//static inline double GetTime()
//{
//  struct timeval tv;
//  gettimeofday(&tv, NULL);
//  return tv.tv_sec + tv.tv_usec / 1e6;
//}
/*
 *
 *
 * begin
 *
 */
void mybplus_tree_init(struct bplus_tree * pg_bplus_tree)
{
    pg_bplus_tree->fd= open("/home/dsj/Desktop/Mbtree/treedata",O_RDWR);
    pg_bplus_tree->offset=0;
}

//bplus_non_leaf
void bplus_non_leaf_reset(struct bplus_non_leaf* pg_bplus_non_leaf){
    int i=0;
    for(i=0;i<BPLUS_MAX_ORDER;i++)
    {
        //childhash[i].clear();
        memset(pg_bplus_non_leaf->childhash, 0, sizeof(pg_bplus_non_leaf->childhash));
    }
}

void bplus_non_leaf_deserialize(struct bplus_non_leaf* pg_bplus_non_leaf, char* buffer)
{
    //cout<<"deserialize"<<endl;
    int64_t pos=0;
    int64_t *tmp=0;
    tmp = (int64_t*)buffer;
    pg_bplus_non_leaf->type=*tmp;
    pos+=sizeof(pg_bplus_non_leaf->type);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_non_leaf->parent_key_idx=*tmp;
    pos+=sizeof(pg_bplus_non_leaf->parent_key_idx);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_non_leaf->children=*tmp;
    pos+=sizeof(pg_bplus_non_leaf->children);

    int i=0;
    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        tmp = (int64_t*)(buffer+pos);
        pg_bplus_non_leaf->key[i] = *tmp;
        pos+=sizeof(int64_t);
    }


    //cout<<"children is: "<<children<<endl;

    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        //childhash[i].assign(buffer+pos,32);
        strncpy(pg_bplus_non_leaf->childhash[i],buffer+pos,32);
        pos+=sizeof(pg_bplus_non_leaf->childhash[i]);
    }

    //node_hash.assign(buffer+pos,32);
    strncpy(pg_bplus_non_leaf->node_hash,buffer+pos,32);
    pos+=sizeof(pg_bplus_non_leaf->node_hash);

    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        tmp = (int64_t*)(buffer+pos);
        pg_bplus_non_leaf->chindrenoffset[i]=*tmp;
        pos+=sizeof(int64_t);
    }

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_non_leaf->node_id=*tmp;
    pos+=sizeof(pg_bplus_non_leaf->node_id);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_non_leaf->parent_id=*tmp;
    pos+=sizeof(pg_bplus_non_leaf->parent_id);
}


void bplus_non_leaf_serialize(struct bplus_non_leaf* pg_bplus_non_leaf, char* buffer)
{
    // cout<<"CHILDREN NUMBER IS: "<<children<<endl;
    //            for(int i=0;i<children;i++)
    //            {
    //                childhash[i]="12345678901234567890123456789012";
    //            }
    int64_t pos=0;
    memcpy(buffer+pos,&pg_bplus_non_leaf->type,sizeof(pg_bplus_non_leaf->type));
    pos+=sizeof(pg_bplus_non_leaf->type);

    memcpy(buffer+pos,&pg_bplus_non_leaf->parent_key_idx,sizeof(pg_bplus_non_leaf->parent_key_idx));
    pos+=sizeof(pg_bplus_non_leaf->parent_key_idx);

    memcpy(buffer+pos,&pg_bplus_non_leaf->children,sizeof(pg_bplus_non_leaf->children));
    pos+=sizeof(pg_bplus_non_leaf->children);

    int i=0;
    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        memcpy(buffer+pos,&pg_bplus_non_leaf->key[i],sizeof(int64_t));
        pos+=sizeof(int64_t);
    }

    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        memcpy(buffer+pos,pg_bplus_non_leaf->childhash[i],sizeof(pg_bplus_non_leaf->childhash[i]));
        pos+=sizeof(pg_bplus_non_leaf->childhash[i]);
//        printf("%s%s\n","`````````````````` : ",pg_bplus_non_leaf->childhash[i]);
    }

    //node_hash="12345678901234567890123456789012";
    memcpy(buffer+pos,pg_bplus_non_leaf->node_hash,sizeof(pg_bplus_non_leaf->node_hash));
    pos+=sizeof(pg_bplus_non_leaf->node_hash);

    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        memcpy(buffer+pos,&pg_bplus_non_leaf->chindrenoffset[i],sizeof(int64_t));
        pos+=sizeof(int64_t);
    }
    memcpy(buffer+pos,&pg_bplus_non_leaf->node_id,sizeof(pg_bplus_non_leaf->node_id));
    pos+=sizeof(pg_bplus_non_leaf->node_id);

    memcpy(buffer+pos,&pg_bplus_non_leaf->parent_id,sizeof(pg_bplus_non_leaf->parent_id));
    pos+=sizeof(pg_bplus_non_leaf->parent_id);

}

int64_t bplus_non_leaf_get_serialize_size(struct bplus_non_leaf* pg_bplus_non_leaf)
{
    int64_t pos=0;
    pos+=sizeof(pg_bplus_non_leaf->type);
    pos+=sizeof(pg_bplus_non_leaf->parent_key_idx);
    pos+=sizeof(pg_bplus_non_leaf->children);
    int i=0;
    for(i=0;i<pg_bplus_non_leaf->children-1;i++)
    {
        pos+=sizeof(int64_t);
    }
    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        pos+=sizeof(pg_bplus_non_leaf->childhash[i]);
    }
    pos+=sizeof(pg_bplus_non_leaf->node_hash);

    for(i=0;i<pg_bplus_non_leaf->children;i++)
    {
        pos+=sizeof(int64_t);
    }
    pos+=sizeof(pg_bplus_non_leaf->node_id);
    pos+=sizeof(pg_bplus_non_leaf->parent_id);
    return pos;
}

void bplus_non_leaf_init(struct bplus_non_leaf* pg_bplus_non_leaf)
{
    int i  = 0;
    pg_bplus_non_leaf->type=-1;
    pg_bplus_non_leaf->parent_key_idx=0;
    pg_bplus_non_leaf->parent = NULL;
    pg_bplus_non_leaf->children=0;
    pg_bplus_non_leaf->node_id=-1;
    pg_bplus_non_leaf->parent_id=-1;
    // pg_bplus_non_leaf->link->prev = NULL;
    // pg_bplus_non_leaf->link->next = NULL;
    for(i = 0; i < BPLUS_MAX_ENTRIES; i++)
    {
        pg_bplus_non_leaf->tids[i] = NULL;
    }
    for(i = 0; i < BPLUS_MAX_ORDER - 1; i++)
    {
        pg_bplus_non_leaf->key[i] = 0;
    }
    for(i = 0; i < BPLUS_MAX_ORDER; i++)
    {
        pg_bplus_non_leaf->chindrenoffset[i] = 0;
    }
}

//bplus_leaf
void bplus_leaf_serialize(struct bplus_leaf* pg_bplus_leaf, char* buffer)
{
    int64_t pos=0;
    memcpy(buffer+pos,&pg_bplus_leaf->type,sizeof(pg_bplus_leaf->type));
    pos+=sizeof(pg_bplus_leaf->type);

    //           for(int i=0;i<entries;i++)
    //           {
    //               childhash[i]="12345678901234567890123456789012";
    //           }

    memcpy(buffer+pos,&pg_bplus_leaf->parent_key_idx,sizeof(pg_bplus_leaf->parent_key_idx));
    pos+=sizeof(pg_bplus_leaf->parent_key_idx);
    memcpy(buffer+pos,&pg_bplus_leaf->entries,sizeof(pg_bplus_leaf->entries));
    pos+=sizeof(pg_bplus_leaf->entries);
    int i=0;
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        memcpy(buffer+pos,&pg_bplus_leaf->key[i],sizeof(int64_t));
        pos+=sizeof(int64_t);
    }
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        memcpy(buffer+pos,pg_bplus_leaf->data[i],sizeof(pg_bplus_leaf->data[i]));
        pos+=sizeof(pg_bplus_leaf->data[i]);
//        printf("%s%s\n","serialize data: ",pg_bplus_leaf->data[i]);
    }
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        memcpy(buffer+pos,pg_bplus_leaf->childhash[i],sizeof(pg_bplus_leaf->childhash[i]));
        pos+=sizeof(pg_bplus_leaf->childhash[i]);
        printf("%s%s\n","serialize leaf childhash : ",pg_bplus_leaf->childhash[i]);
    }
    //node_hash = "12345678901234567890123456789012";
    memcpy(buffer+pos,pg_bplus_leaf->node_hash,sizeof(pg_bplus_leaf->node_hash));
    pos+=sizeof(pg_bplus_leaf->node_hash);

    memcpy(buffer+pos,&pg_bplus_leaf->node_id,sizeof(pg_bplus_leaf->node_id));
    pos+=sizeof(pg_bplus_leaf->node_id);

    memcpy(buffer+pos,&pg_bplus_leaf->parent_id,sizeof(pg_bplus_leaf->parent_id));
    pos+=sizeof(pg_bplus_leaf->parent_id);

    //added pointer
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        //序列化BlockIdData
        memcpy(buffer+pos, &pg_bplus_leaf->tids[i]->ip_blkid.bi_hi, sizeof(uint16));
        pos+=sizeof(uint16);
        memcpy(buffer+pos, &pg_bplus_leaf->tids[i]->ip_blkid.bi_lo, sizeof(uint16));
        pos+=sizeof(uint16);
        //序列化OffsetNumber
        memcpy(buffer+pos, &pg_bplus_leaf->tids[i]->ip_posid, sizeof(uint16));
        pos+=sizeof(uint16);
    }

}

void bplus_leaf_deserialize(bplus_leaf* pg_bplus_leaf, char* buffer)
{
    int64_t pos=0;
    int64_t *tmp=0;
    tmp = (int64_t*)buffer;
    pg_bplus_leaf->type=*tmp;
    pos+=sizeof(pg_bplus_leaf->type);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_leaf->parent_key_idx=*tmp;
    pos+=sizeof(pg_bplus_leaf->parent_key_idx);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_leaf->entries=*tmp;
    pos+=sizeof(pg_bplus_leaf->entries);

    int i=0;
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        tmp = (int64_t*)(buffer+pos);
        pg_bplus_leaf->key[i] = *tmp;
        pos+=sizeof(int64_t);
    }
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        //childhash[i].assign(buffer+pos,32);
        strncpy(pg_bplus_leaf->data[i],buffer+pos, DATA_SIZE);
        pos+=sizeof(pg_bplus_leaf->data[i]);
        printf("%s%s\n","deserialize data: ",pg_bplus_leaf->data[i]);
    }
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        //childhash[i].assign(buffer+pos,32);
        strncpy(pg_bplus_leaf->childhash[i],buffer+pos,32);
//            char* bufadd=strcat(buffer,(char*)pos);
//            memset(childhash[i], bufadd, 32);
        pos+=sizeof(pg_bplus_leaf->childhash[i]);
        printf("%s%s\n","deserialize childhash : ",pg_bplus_leaf->childhash[i]);
    }

    //node_hash.assign(buffer+pos,32);
    strncpy(pg_bplus_leaf->node_hash,buffer+pos,32);
    //memset(node_hash, buffer+(char*)pos, 32);
    pos+=sizeof(pg_bplus_leaf->node_hash);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_leaf->node_id=*tmp;
    pos+=sizeof(pg_bplus_leaf->node_id);

    tmp = (int64_t*)(buffer+pos);
    pg_bplus_leaf->parent_id=*tmp;
    pos+=sizeof(pg_bplus_leaf->parent_id);


    uint16 *tmpp;
    //added pointer
    for(i=0;i<pg_bplus_leaf->entries;i++){
        pg_bplus_leaf->tids[i] = (ItemPointer)malloc(sizeof(ItemPointerData));
        tmpp = (uint16*)(buffer+pos);
        pg_bplus_leaf->tids[i]->ip_blkid.bi_hi = *tmpp;
        pos+=sizeof(pg_bplus_leaf->tids[i]->ip_blkid.bi_hi);

        tmpp = (uint16*)(buffer+pos);
        pg_bplus_leaf->tids[i]->ip_blkid.bi_lo = *tmpp;
        pos+=sizeof(pg_bplus_leaf->tids[i]->ip_blkid.bi_lo);

        tmpp = (uint16*)(buffer+pos);
        pg_bplus_leaf->tids[i]->ip_posid= *tmpp;
        pos+=sizeof(pg_bplus_leaf->tids[i]->ip_posid);
    }
}

int64_t bplus_leaf_get_serialize_size(struct bplus_leaf* pg_bplus_leaf)
{
    int64_t pos=0;

    pos+=sizeof(pg_bplus_leaf->type);

    pos+=sizeof(pg_bplus_leaf->parent_key_idx);

    pos+=sizeof(pg_bplus_leaf->entries);

    int i=0;
    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        pos+=sizeof(int64_t);
    }

    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        pos+=sizeof(pg_bplus_leaf->data[i]);
    }

    for(i=0;i<pg_bplus_leaf->entries;i++)
    {
        pos+=sizeof(pg_bplus_leaf->childhash[i]);
    }
    pos+=sizeof(pg_bplus_leaf->node_hash);
    pos+=sizeof(pg_bplus_leaf->node_id);
    pos+=sizeof(pg_bplus_leaf->parent_id);
    return pos;
}

void bplus_leaf_init(struct bplus_leaf* pg_bplus_leaf)
{
    pg_bplus_leaf->type=-1;
    pg_bplus_leaf->parent_key_idx=0;
    pg_bplus_leaf->parent = NULL;
    pg_bplus_leaf->node_id=-1;
    pg_bplus_leaf->parent_id=-1;

}

//rangevo
void rangevo_init(struct rangevo* pg_rangevo,int hashmap_size){
    int i = 0;
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        vector_setup(&pg_rangevo->leftvo[i].nhash, 60, 33);
        vector_setup(&pg_rangevo->rightvo[i].nhash, 60, 33);
    }
    pg_rangevo->traverse_height = 0;
    vector_setup(&pg_rangevo->data, 60, sizeof(pgvector));
    vector_setup(&pg_rangevo->itemdata, 60, sizeof(pgvector));
    pg_rangevo->hashmap = (char*)malloc(hashmap_size*sizeof(char));
    memset(pg_rangevo->hashmap, 0, hashmap_size);
    pg_rangevo->mbtree_size = hashmap_size / 8192;

//    vector_setup(&pg_rangevo->result, 60, sizeof(pgvector));
}

void vo_serialize(rangevo* vo, char* buffer){
    int i = 0, j = 0, k = 0;
    int64_t pos=0;
    memcpy(buffer+pos, &vo->traverse_height, sizeof(vo->traverse_height));
    pos+=sizeof(vo->traverse_height);
    //serialize the vo.data
    size_t data_size = vo->data.size;
    memcpy(buffer+pos, &vo->data.size, sizeof(vo->data.size));
    pos+=sizeof(vo->data.size);
    for(i = 0; i < data_size; i++){
        pgvector myvector1 = *(pgvector*)vector_get(&vo->data, i);
        size_t data_size1 = myvector1.size;
        memcpy(buffer+pos, &myvector1.size, sizeof(myvector1.size));
        pos+=sizeof(myvector1.size);
        for(j = 0; j < data_size1; j++){
            char* result = *(char**)vector_get(&myvector1, j);
            size_t len = strlen(result);
            memcpy(buffer+pos, &len, sizeof(len));
            pos+=sizeof(len);
            memcpy(buffer+pos, result, len);
            pos += len;
//            free(result);
//            pos+=sizeof(pg_bplus_non_leaf->childhash[i]);
        }
//        vector_destroy(&myvector1);
    }
//    vector_destroy(&vo->data);
    //serialize the vo.leftvo
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        nodevo my_node = vo->leftvo[i];
        memcpy(buffer+pos, &my_node.node_id, sizeof(int64_t));
        pos+=sizeof(int64_t);
        memcpy(buffer+pos, &my_node.bound_node_childno, sizeof(int64_t));
        pos+=sizeof(int64_t);
        size_t data_size1 = my_node.nhash.size;
        memcpy(buffer+pos, &my_node.nhash.size, sizeof(my_node.nhash.size));
        pos+=sizeof(my_node.nhash.size);
        for(j = 0; j < data_size1; j++){
            char* temp_hash = (char*)vector_get(&my_node.nhash, j);
            memcpy(buffer+pos, temp_hash, 32);
            pos+=32;
//            free(temp_hash);
        }
    }

    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        nodevo my_node = vo->rightvo[i];
        memcpy(buffer+pos, &my_node.node_id, sizeof(int64_t));
        pos+=sizeof(int64_t);
        memcpy(buffer+pos, &my_node.bound_node_childno, sizeof(int64_t));
        pos+=sizeof(int64_t);
        size_t data_size1 = my_node.nhash.size;
        memcpy(buffer+pos, &my_node.nhash.size, sizeof(my_node.nhash.size));
        pos+=sizeof(my_node.nhash.size);
        for(j = 0; j < data_size1; j++){
            char* temp_hash = (char*)vector_get(&my_node.nhash, j);
            memcpy(buffer+pos, temp_hash, DATA_SIZE);
            pos+=DATA_SIZE;
//            free(temp_hash);
        }
    }
    printf("-------");
}

void vo_deserialize(rangevo* vo, char* buffer){
    int i = 0, j = 0, k = 0;
    int64_t pos=0;
    int64_t *tmp=0;
    int *itmp;
    size_t  *tmp_size = 0;
    itmp = (int*)(buffer+pos);
    vo->traverse_height = *itmp;
    pos+=sizeof(vo->traverse_height);
    tmp_size = (size_t*)(buffer+pos);
    size_t data_size = *tmp_size;
    pos+=sizeof(data_size);
//    pgvector* pv = (pgvector*)malloc(sizeof(pgvector));
//    vector_setup(pv, 60, sizeof(pgvector));
    for(i = 0; i < data_size; i++){
         pgvector* subpv = (pgvector*)malloc(sizeof(pgvector));
         vector_setup(subpv, 60, DATA_SIZE+1);
         tmp_size = (size_t*)(buffer+pos);
         size_t data_size1 = *tmp_size;
         pos+=sizeof(data_size1);
         for(j = 0; j < data_size1; j++){
             tmp_size = (size_t*)(buffer+pos);
             size_t len = *tmp_size;
             pos+=sizeof(len);
             char *value = (char*)malloc(sizeof(char) * (len+1));
             memset(value, 0, len+1);
             strncpy(value, buffer+pos, len);
             pos+=len;
             vector_push_back(subpv, &value);
             char* tmp_id= *(char**)vector_get(subpv, j);
         }
         vector_push_back(&vo->data, subpv);
    }

    //deserialize the vo.leftvo
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        tmp = (int64_t*)(buffer+pos);
        vo->leftvo[i].node_id = *tmp;
        pos+=sizeof(vo->leftvo[i].node_id);
        tmp = (int64_t*)(buffer+pos);
        vo->leftvo[i].bound_node_childno = *tmp;
        pos+=sizeof(vo->leftvo[i].bound_node_childno);
        tmp_size = (size_t*)(buffer+pos);
        size_t pv_size = *tmp_size;
        pos+=sizeof(pv_size);
        for(j = 0; j < pv_size; j++){
             char *value = (char*)malloc(sizeof(char) * (DATA_SIZE+1));
             memset(value, 0, DATA_SIZE+1);
             strncpy(value, buffer+pos, DATA_SIZE);
             pos+=DATA_SIZE;
             vector_push_back(&vo->leftvo[i].nhash, value);
        }
    }

    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        tmp = (int64_t*)(buffer+pos);
        vo->rightvo[i].node_id = *tmp;
        pos+=sizeof(vo->rightvo[i].node_id);
        tmp = (int64_t*)(buffer+pos);
        vo->rightvo[i].bound_node_childno = *tmp;
        pos+=sizeof(vo->rightvo[i].bound_node_childno);
        tmp_size = (size_t*)(buffer+pos);
        size_t pv_size = *tmp_size;
        pos+=sizeof(pv_size);
        for(j = 0; j < pv_size; j++){
             char *value = (char*)malloc(sizeof(char) * (DATA_SIZE+1));
             memset(value, 0, DATA_SIZE+1);
             strncpy(value, buffer+pos, DATA_SIZE);
             pos+=DATA_SIZE;
             vector_push_back(&vo->rightvo[i].nhash, value);
        }
    }
}


static key_bt key_binary_search(key_bt *arr, int64_t len, key_bt target)
{
    int64_t low = -1;
    int64_t high = len;
    while (low + 1 < high) {
        int64_t mid = low + (high - low) / 2;
        if (target > arr[mid]) {
            low = mid;
        } else {
            high = mid;
        }
    }
    if (high >= len || arr[high] != target) {
        return -high - 1;
    } else {
        return high;
    }
}

//int64_t bplus_tree_get_range(struct bplus_tree *tree, key_bt key1, key_bt key2,vector<int64_t> *result)
//{
//    int64_t i, data = 0;
//    key_bt min = key1 <= key2 ? key1 : key2;
//    key_bt max = min == key1 ? key2 : key1;
//    struct bplus_node *node = tree->root;

//    while (node != NULL) {
//        if (is_leaf(node)) {
//            struct bplus_leaf *ln = (struct bplus_leaf *)node;
//            i = key_binary_search(ln->key, ln->entries, min);
//            if (i < 0) {
//                i = -i - 1;
//                if (i >= ln->entries) {
//                    ln = list_next_entry(ln, link);
//                }
//            }
//            while (ln != NULL && ln->key[i] <= max) {

//                data = ln->data[i];
//                result->push_back(data);
//                if (++i >= ln->entries) {
//                    ln = list_next_entry(ln, link);
//                    i = 0;
//                }
//            }
//            break;
//        } else {
//            struct bplus_non_leaf *nln = (struct bplus_non_leaf *)node;
//            i = key_binary_search(nln->key, nln->children - 1, min);
//            if (i >= 0) {
//                node = nln->sub_ptr[i + 1];
//            } else  {
//                i = -i - 1;
//                node = nln->sub_ptr[i];
//            }
//        }
//    }

//    return data;
//}

static struct bplus_non_leaf *non_leaf_new(void)
{
    struct bplus_non_leaf *node = (bplus_non_leaf*)malloc(sizeof(bplus_non_leaf));
    bplus_non_leaf_init(node);
    assert(node != NULL);
    list_init(&node->link);
    node->type = BPLUS_TREE_NON_LEAF;
    node->parent_key_idx = -1;
    return node;
}

static struct bplus_leaf *leaf_new(void)
{
    struct bplus_leaf *node = (bplus_leaf*)malloc(sizeof(bplus_leaf));
    bplus_leaf_init(node);
    assert(node != NULL);
    list_init(&node->link);
    node->type = BPLUS_TREE_LEAF;
    node->parent_key_idx = -1;
    int i = 0;
    for(i < 0; i < BPLUS_MAX_ENTRIES; i++){
        node->tids[i] = (ItemPointer)malloc(sizeof(ItemPointerData));
    }
    return node;
}

static void non_leaf_delete(struct bplus_non_leaf *node)
{
    list_del(&node->link);
    free(node);
}

static void leaf_delete(struct bplus_leaf *node)
{
    list_del(&node->link);
    free(node);
}

static char* bplus_tree_search(struct bplus_tree *tree, key_bt key)
{
    int64_t i = -1;
    char *ret = (char*)malloc(32);
    strcpy(ret,"-1");
    int64_t height = 0;
    struct bplus_node *node = tree->root;
    while (node != NULL) {
        if (is_leaf(node)) {
            struct bplus_leaf *ln = (struct bplus_leaf *)node;
            i = key_binary_search(ln->key, ln->entries, key);
            //ret = i >= 0 ? ln->data[i] : "0";
            if(i >= 0){
                strcpy(ret,ln->data[i]);
            }else{
                strcpy(ret,"0");
            }
            break;
        } else {
            struct bplus_non_leaf *nln = (struct bplus_non_leaf *)node;
            bplus_non_leaf_init(nln);
            i = key_binary_search(nln->key, nln->children - 1, key);
            if (i >= 0) {
                node = nln->sub_ptr[i + 1];
                //                                cout<<"search height: "<<height<<"*****************************/"<<endl;
                //                                cout<<"hit key vale: "<<nln->key[i+1]<<endl;
                height++;
            } else {

                i = -i - 1;
                node = nln->sub_ptr[i];
                //                                cout<<"search height: "<<height<<"*****************************/"<<endl;
                //                                cout<<"hit key vale: "<<nln->key[i]<<endl;
                height++;
            }
        }
    }
    return ret;
}




static int64_t non_leaf_insert(struct bplus_tree *tree, struct bplus_non_leaf *node,
                               struct bplus_node *l_ch, struct bplus_node *r_ch, key_bt key, int64_t level);

static int64_t parent_node_build(struct bplus_tree *tree, struct bplus_node *left,
                                 struct bplus_node *right, key_bt key, int64_t level)
{
    if (left->parent == NULL && right->parent == NULL) {
        /* new parent */
        struct bplus_non_leaf *parent = non_leaf_new();
        parent->key[0] = key;
        parent->sub_ptr[0] = left;
        parent->sub_ptr[0]->parent = parent;
        parent->sub_ptr[0]->parent_key_idx = -1;
        parent->sub_ptr[1] = right;
        parent->sub_ptr[1]->parent = parent;
        parent->sub_ptr[1]->parent_key_idx = 0;
        parent->children = 2;
        /* update root */
        tree->root = (struct bplus_node *)parent;
        list_add(&parent->link, &tree->list[++tree->level]);
        return 0;
    } else if (right->parent == NULL) {
        /* trace upwards */
        right->parent = left->parent;
        return non_leaf_insert(tree, left->parent, left, right, key, level + 1);
    } else {
        /* trace upwards */
        left->parent = right->parent;
        return non_leaf_insert(tree, right->parent, left, right, key, level + 1);
    }
}

static int64_t non_leaf_split_left(struct bplus_non_leaf *node, struct bplus_non_leaf *left,
                                   struct bplus_node *l_ch, struct bplus_node *r_ch, key_bt key, int64_t insert)
{
    int64_t i, j, order = node->children;
    key_bt split_key;
    /* split = [m/2] */
    int64_t split = (order + 1) / 2;
    /* split as left sibling */
    __list_add(&left->link, node->link.prev, &node->link);
    /* replicate from sub[0] to sub[split - 1] */
    for (i = 0, j = 0; i < split; i++, j++) {
        if (j == insert) {
            left->sub_ptr[j] = l_ch;
            left->sub_ptr[j]->parent = left;
            left->sub_ptr[j]->parent_key_idx = j - 1;
            left->sub_ptr[j + 1] = r_ch;
            left->sub_ptr[j + 1]->parent = left;
            left->sub_ptr[j + 1]->parent_key_idx = j;
            j++;
        } else {
            left->sub_ptr[j] = node->sub_ptr[i];
            left->sub_ptr[j]->parent = left;
            left->sub_ptr[j]->parent_key_idx = j - 1;
        }
    }
    left->children = split;
    /* replicate from key[0] to key[split - 2] */
    for (i = 0, j = 0; i < split - 1; j++) {
        if (j == insert) {
            left->key[j] = key;
        } else {
            left->key[j] = node->key[i];
            i++;
        }
    }
    if (insert == split - 1) {
        left->key[insert] = key;
        left->sub_ptr[insert] = l_ch;
        left->sub_ptr[insert]->parent = left;
        left->sub_ptr[insert]->parent_key_idx = j - 1;
        node->sub_ptr[0] = r_ch;
        split_key = key;
    } else {
        node->sub_ptr[0] = node->sub_ptr[split - 1];
        split_key = node->key[split - 2];
    }
    node->sub_ptr[0]->parent = node;
    node->sub_ptr[0]->parent_key_idx = - 1;
    /* left shift for right node from split - 1 to children - 1 */
    for (i = split - 1, j = 0; i < order - 1; i++, j++) {
        node->key[j] = node->key[i];
        node->sub_ptr[j + 1] = node->sub_ptr[i + 1];
        node->sub_ptr[j + 1]->parent = node;
        node->sub_ptr[j + 1]->parent_key_idx = j;
    }
    node->sub_ptr[j] = node->sub_ptr[i];
    node->children = j + 1;
    return split_key;
}

static int64_t non_leaf_split_right1(struct bplus_non_leaf *node, struct bplus_non_leaf *right,
                                     struct bplus_node *l_ch, struct bplus_node *r_ch, key_bt key, int64_t insert)
{
    int64_t i, j, order = node->children;
    key_bt split_key;
    /* split = [m/2] */
    int64_t split = (order + 1) / 2;
    /* split as right sibling */
    list_add(&right->link, &node->link);
    /* split key is key[split - 1] */
    split_key = node->key[split - 1];
    /* left node's children always be [split] */
    node->children = split;
    /* right node's first sub-node */
    right->key[0] = key;
    right->sub_ptr[0] = l_ch;
    right->sub_ptr[0]->parent = right;
    right->sub_ptr[0]->parent_key_idx = -1;
    right->sub_ptr[1] = r_ch;
    right->sub_ptr[1]->parent = right;
    right->sub_ptr[1]->parent_key_idx = 0;
    /* insertion point64_t is split point64_t, replicate from key[split] */
    for (i = split, j = 1; i < order - 1; i++, j++) {
        right->key[j] = node->key[i];
        right->sub_ptr[j + 1] = node->sub_ptr[i + 1];
        right->sub_ptr[j + 1]->parent = right;
        right->sub_ptr[j + 1]->parent_key_idx = j;
    }
    right->children = j + 1;
    return split_key;
}

static int64_t non_leaf_split_right2(struct bplus_non_leaf *node, struct bplus_non_leaf *right,
                                     struct bplus_node *l_ch, struct bplus_node *r_ch, key_bt key, int64_t insert)
{
    int64_t i, j, order = node->children;
    key_bt split_key;
    /* split = [m/2] */
    int64_t split = (order + 1) / 2;
    /* left node's children always be [split + 1] */
    node->children = split + 1;
    /* split as right sibling */
    list_add(&right->link, &node->link);
    /* split key is key[split] */
    split_key = node->key[split];
    /* right node's first sub-node */
    right->sub_ptr[0] = node->sub_ptr[split + 1];
    right->sub_ptr[0]->parent = right;
    right->sub_ptr[0]->parent_key_idx = -1;
    /* replicate from key[split + 1] to key[order - 1] */
    for (i = split + 1, j = 0; i < order - 1; j++) {
        if (j != insert - split - 1) {
            right->key[j] = node->key[i];
            right->sub_ptr[j + 1] = node->sub_ptr[i + 1];
            right->sub_ptr[j + 1]->parent = right;
            right->sub_ptr[j + 1]->parent_key_idx = j;
            i++;
        }
    }
    /* reserve a hole for insertion */
    if (j > insert - split - 1) {
        right->children = j + 1;
    } else {
        assert(j == insert - split - 1);
        right->children = j + 2;
    }
    /* insert new key and sub-node */
    j = insert - split - 1;
    right->key[j] = key;
    right->sub_ptr[j] = l_ch;
    right->sub_ptr[j]->parent = right;
    right->sub_ptr[j]->parent_key_idx = j - 1;
    right->sub_ptr[j + 1] = r_ch;
    right->sub_ptr[j + 1]->parent = right;
    right->sub_ptr[j + 1]->parent_key_idx = j;
    return split_key;
}

static void non_leaf_simple_insert(struct bplus_non_leaf *node, struct bplus_node *l_ch,
                                   struct bplus_node *r_ch, key_bt key, int64_t insert)
{
    int64_t i;
    for (i = node->children - 1; i > insert; i--) {
        node->key[i] = node->key[i - 1];
        node->sub_ptr[i + 1] = node->sub_ptr[i];
        node->sub_ptr[i + 1]->parent_key_idx = i;
    }
    node->key[i] = key;
    node->sub_ptr[i] = l_ch;
    node->sub_ptr[i]->parent_key_idx = i - 1;
    node->sub_ptr[i + 1] = r_ch;
    node->sub_ptr[i + 1]->parent_key_idx = i;
    node->children++;
}

static int64_t non_leaf_insert(struct bplus_tree *tree, struct bplus_non_leaf *node,
                               struct bplus_node *l_ch, struct bplus_node *r_ch, key_bt key, int64_t level)
{
    /* search key location */
    int64_t insert = key_binary_search(node->key, node->children - 1, key);
    assert(insert < 0);
    insert = -insert - 1;

    /* node is full */
    if (node->children == tree->order) {
        /* split = [m/2] */
        key_bt split_key;
        int64_t split = (node->children + 1) / 2;
        struct bplus_non_leaf *sibling = non_leaf_new();
        if (insert < split) {
            split_key = non_leaf_split_left(node, sibling, l_ch, r_ch, key, insert);
        } else if (insert == split) {
            split_key = non_leaf_split_right1(node, sibling, l_ch, r_ch, key, insert);
        } else {
            split_key = non_leaf_split_right2(node, sibling, l_ch, r_ch, key, insert);
        }
        /* build new parent */
        if (insert < split) {
            return parent_node_build(tree, (struct bplus_node *)sibling,
                                     (struct bplus_node *)node, split_key, level);
        } else {
            return parent_node_build(tree, (struct bplus_node *)node,
                                     (struct bplus_node *)sibling, split_key, level);
        }
    } else {
        non_leaf_simple_insert(node, l_ch, r_ch, key, insert);
    }

    return 0;
}

static void leaf_split_left(struct bplus_leaf *leaf, struct bplus_leaf *left,
                            key_bt key, char* data, ItemPointer tid, int64_t insert)
{
//    memset(data,0,32);
    int64_t i, j;
    /* split = [m/2] */
    int64_t split = (leaf->entries + 1) / 2;
    /* split as left sibling */
    __list_add(&left->link, leaf->link.prev, &leaf->link);
    /* replicate from 0 to key[split - 2] */
    for (i = 0, j = 0; i < split - 1; j++) {
        if (j == insert) {
            left->key[j] = key;
            //left->data[j] = data;
            strcpy(left->data[j] , data);
            left->tids[j]->ip_blkid.bi_hi = tid->ip_blkid.bi_hi;
            left->tids[j]->ip_blkid.bi_lo = tid->ip_blkid.bi_lo;
            left->tids[j]->ip_posid= tid->ip_posid;
        } else {
            left->key[j] = leaf->key[i];
            //left->data[j] = leaf->data[i];
            strcpy(left->data[j] , leaf->data[i]);
            left->tids[j] = leaf->tids[i];
            i++;
        }
    }
    if (j == insert) {
        left->key[j] = key;
        //left->data[j] = data;
        strcpy(left->data[j] , data);
//        left->tids[j] = tid;
        left->tids[j]->ip_blkid.bi_hi = tid->ip_blkid.bi_hi;
        left->tids[j]->ip_blkid.bi_lo = tid->ip_blkid.bi_lo;
        left->tids[j]->ip_posid= tid->ip_posid;
        j++;
    }
    left->entries = j;
    /* left shift for right node */
    for (j = 0; i < leaf->entries; i++, j++) {
        leaf->key[j] = leaf->key[i];
        leaf->tids[j] = leaf->tids[i];
        //leaf->data[j] = leaf->data[i];
        strcpy(left->data[j] , leaf->data[i]);

    }
    leaf->entries = j;
}

static void leaf_split_right(struct bplus_leaf *leaf, struct bplus_leaf *right,
                             key_bt key, char* data, ItemPointer tid, int64_t insert)
{
//    memset(data,0,32);
    int64_t i, j;
    /* split = [m/2] */
    int64_t split = (leaf->entries + 1) / 2;
    /* split as right sibling */
    list_add(&right->link, &leaf->link);
    /* replicate from key[split] */
    for (i = split, j = 0; i < leaf->entries; j++) {
        if (j != insert - split) {
            right->key[j] = leaf->key[i];
            right->tids[j] = leaf->tids[i];
            //right->data[j] = leaf->data[i];
            strcpy(right->data[j] , leaf->data[i]);
            i++;
        }
    }
    /* reserve a hole for insertion */
    if (j > insert - split) {
        right->entries = j;
    } else {
        assert(j == insert - split);
        right->entries = j + 1;
    }
    /* insert new key */
    j = insert - split;
    right->key[j] = key;
    right->tids[j]->ip_blkid.bi_hi = tid->ip_blkid.bi_hi;
    right->tids[j]->ip_blkid.bi_lo = tid->ip_blkid.bi_lo;
    right->tids[j]->ip_posid= tid->ip_posid;
//    right->data[j] = data;
    strcpy(right->data[j] , data);
    /* left leaf number */
    leaf->entries = split;
}

static void leaf_simple_insert(struct bplus_leaf *leaf, key_bt key, char* data, ItemPointer tid, int64_t insert)
{
    //memset(data,0,32);
    int64_t i;
    for (i = leaf->entries; i > insert; i--) {
        leaf->key[i] = leaf->key[i - 1];
        leaf->tids[i] = leaf->tids[i - 1];
//        leaf->data[i] = leaf->data[i - 1];
        strcpy(leaf->data[i] , leaf->data[i - 1]);
    }
    leaf->key[i] = key;
    leaf->tids[i]->ip_blkid.bi_hi = tid->ip_blkid.bi_hi;
    leaf->tids[i]->ip_blkid.bi_lo = tid->ip_blkid.bi_lo;
    leaf->tids[i]->ip_posid= tid->ip_posid;
//    leaf->data[i] = data;
    strcpy(leaf->data[i] , data);
    leaf->entries++;
}

static int64_t leaf_insert(struct bplus_tree *tree, struct bplus_leaf *leaf, key_bt key, char* data, ItemPointer tid)
{
    /* search key location */
    int64_t insert = key_binary_search(leaf->key, leaf->entries, key);
    if (insert >= 0) {
        /* Already exists */
        return -1;
    }
    insert = -insert - 1;

    /* node full */
    //the entries in leaf node are equal to the size of "entries"
    if (leaf->entries == tree->entries) {
        /* split = [m/2] */
        int64_t split = (tree->entries + 1) / 2;
        /* splited sibling node */
        struct bplus_leaf *sibling = leaf_new();
        /* sibling leaf replication due to location of insertion */
        if (insert < split) {
            leaf_split_left(leaf, sibling, key, data, tid, insert);
        } else {
            leaf_split_right(leaf, sibling, key, data, tid, insert);
        }
        /* build new parent */
        if (insert < split) {
            return parent_node_build(tree, (struct bplus_node *)sibling,
                                     (struct bplus_node *)leaf, leaf->key[0], 0);
        } else {
            return parent_node_build(tree, (struct bplus_node *)leaf,
                                     (struct bplus_node *)sibling, sibling->key[0], 0);
        }
    }
        //the number of key equals to data at leaf level
    else
    {
        leaf_simple_insert(leaf, key, data, tid, insert);
    }

    return 0;
}

static int64_t bplus_tree_insert(struct bplus_tree *tree, key_bt key, char* data, ItemPointer tid)
{
    struct bplus_node *node = tree->root;
    while (node != NULL) {
        if (is_leaf(node)) {
            struct bplus_leaf *ln = (struct bplus_leaf *)node;
            return leaf_insert(tree, ln, key, data, tid);
        } else {
            struct bplus_non_leaf *nln = (struct bplus_non_leaf *)node;
            int64_t i = key_binary_search(nln->key, nln->children - 1, key);
            if (i >= 0) {
                node = nln->sub_ptr[i + 1];
            } else {
                i = -i - 1;
                node = nln->sub_ptr[i];
            }
        }
    }

    /* new root */
    struct bplus_leaf *root = leaf_new();
    root->key[0] = key;
    root->tids[0]->ip_blkid.bi_hi = tid->ip_blkid.bi_hi;
    root->tids[0]->ip_blkid.bi_lo = tid->ip_blkid.bi_lo;
    root->tids[0]->ip_posid= tid->ip_posid;
//    root->data[0] = data;
    strcpy(root->data[0] , data);
    root->entries = 1;
    tree->root = (struct bplus_node *)root;
    list_add(&root->link, &tree->list[tree->level]);
    return 0;
}

static int64_t non_leaf_sibling_select(struct bplus_non_leaf *l_sib, struct bplus_non_leaf *r_sib,
                                       struct bplus_non_leaf *parent, int64_t i)
{
    if (i == -1) {
        /* the frist sub-node, no left sibling, choose the right one */
        return RIGHT_SIBLING;
    } else if (i == parent->children - 2) {
        /* the last sub-node, no right sibling, choose the left one */
        return LEFT_SIBLING;
    } else {
        /* if both left and right sibling found, choose the one with more entries */
        return l_sib->children >= r_sib->children ? LEFT_SIBLING : RIGHT_SIBLING;
    }
}

static void non_leaf_shift_from_left(struct bplus_non_leaf *node, struct bplus_non_leaf *left,
                                     int64_t parent_key_index, int64_t remove)
{
    int64_t i;
    /* node's elements right shift */
    for (i = remove; i > 0; i--) {
        node->key[i] = node->key[i - 1];
    }
    for (i = remove + 1; i > 0; i--) {
        node->sub_ptr[i] = node->sub_ptr[i - 1];
        node->sub_ptr[i]->parent_key_idx = i - 1;
    }
    /* parent key right rotation */
    node->key[0] = node->parent->key[parent_key_index];
    node->parent->key[parent_key_index] = left->key[left->children - 2];
    /* borrow the last sub-node from left sibling */
    node->sub_ptr[0] = left->sub_ptr[left->children - 1];
    node->sub_ptr[0]->parent = node;
    node->sub_ptr[0]->parent_key_idx = -1;
    left->children--;
}

static void non_leaf_merge_int64_to_left(struct bplus_non_leaf *node, struct bplus_non_leaf *left,
                                         int64_t parent_key_index, int64_t remove)
{
    int64_t i, j;
    /* move parent key down */
    left->key[left->children - 1] = node->parent->key[parent_key_index];
    /* merge int64_to left sibling */
    for (i = left->children, j = 0; j < node->children - 1; j++) {
        if (j != remove) {
            left->key[i] = node->key[j];
            i++;
        }
    }
    for (i = left->children, j = 0; j < node->children; j++) {
        if (j != remove + 1) {
            left->sub_ptr[i] = node->sub_ptr[j];
            left->sub_ptr[i]->parent = left;
            left->sub_ptr[i]->parent_key_idx = i - 1;
            i++;
        }
    }
    left->children = i;
    /* delete empty node */
    non_leaf_delete(node);
}

static void non_leaf_shift_from_right(struct bplus_non_leaf *node, struct bplus_non_leaf *right,
                                      int64_t parent_key_index)
{
    int64_t i;
    /* parent key left rotation */
    node->key[node->children - 1] = node->parent->key[parent_key_index];
    node->parent->key[parent_key_index] = right->key[0];
    /* borrow the frist sub-node from right sibling */
    node->sub_ptr[node->children] = right->sub_ptr[0];
    node->sub_ptr[node->children]->parent = node;
    node->sub_ptr[node->children]->parent_key_idx = node->children - 1;
    node->children++;
    /* left shift in right sibling */
    for (i = 0; i < right->children - 2; i++) {
        right->key[i] = right->key[i + 1];
    }
    for (i = 0; i < right->children - 1; i++) {
        right->sub_ptr[i] = right->sub_ptr[i + 1];
        right->sub_ptr[i]->parent_key_idx = i - 1;
    }
    right->children--;
}

static void non_leaf_merge_from_right(struct bplus_non_leaf *node, struct bplus_non_leaf *right,
                                      int64_t parent_key_index)
{
    int64_t i, j;
    /* move parent key down */
    node->key[node->children - 1] = node->parent->key[parent_key_index];
    node->children++;
    /* merge from right sibling */
    for (i = node->children - 1, j = 0; j < right->children - 1; i++, j++) {
        node->key[i] = right->key[j];
    }
    for (i = node->children - 1, j = 0; j < right->children; i++, j++) {
        node->sub_ptr[i] = right->sub_ptr[j];
        node->sub_ptr[i]->parent = node;
        node->sub_ptr[i]->parent_key_idx = i - 1;
    }
    node->children = i;
    /* delete empty right sibling */
    non_leaf_delete(right);
}

static void non_leaf_simple_remove(struct bplus_non_leaf *node, int64_t remove)
{
    assert(node->children >= 2);
    for (; remove < node->children - 2; remove++) {
        node->key[remove] = node->key[remove + 1];
        node->sub_ptr[remove + 1] = node->sub_ptr[remove + 2];
        node->sub_ptr[remove + 1]->parent_key_idx = remove;
    }
    node->children--;
}

static void non_leaf_remove(struct bplus_tree *tree, struct bplus_non_leaf *node, int64_t remove)
{
    if (node->children <= (tree->order + 1) / 2) {
        struct bplus_non_leaf *l_sib = list_prev_entry(node, link);
        struct bplus_non_leaf *r_sib = list_next_entry(node, link);
        struct bplus_non_leaf *parent = node->parent;
        if (parent != NULL) {
            /* decide which sibling to be borrowed from */
            int64_t i = node->parent_key_idx;
            if (non_leaf_sibling_select(l_sib, r_sib, parent, i) == LEFT_SIBLING) {
                if (l_sib->children > (tree->order + 1) / 2) {
                    non_leaf_shift_from_left(node, l_sib, i, remove);
                } else {
                    non_leaf_merge_int64_to_left(node, l_sib, i, remove);
                    /* trace upwards */
                    non_leaf_remove(tree, parent, i);
                }
            } else {
                /* remove first in case of overflow during merging with sibling */
                non_leaf_simple_remove(node, remove);
                if (r_sib->children > (tree->order + 1) / 2) {
                    non_leaf_shift_from_right(node, r_sib, i + 1);
                } else {
                    non_leaf_merge_from_right(node, r_sib, i + 1);
                    /* trace upwards */
                    non_leaf_remove(tree, parent, i + 1);
                }
            }
        } else {
            if (node->children == 2) {
                /* delete old root node */
                assert(remove == 0);
                node->sub_ptr[0]->parent = NULL;
                tree->root = node->sub_ptr[0];
                non_leaf_delete(node);
                tree->level--;
            } else {
                non_leaf_simple_remove(node, remove);
            }
        }
    } else {
        non_leaf_simple_remove(node, remove);
    }
}

static int64_t leaf_sibling_select(struct bplus_leaf *l_sib, struct bplus_leaf *r_sib,
                                   struct bplus_non_leaf *parent, int64_t i)
{
    if (i == -1) {
        /* the frist sub-node, no left sibling, choose the right one */
        return RIGHT_SIBLING;
    } else if (i == parent->children - 2) {
        /* the last sub-node, no right sibling, choose the left one */
        return LEFT_SIBLING;
    } else {
        /* if both left and right sibling found, choose the one with more entries */
        return l_sib->entries >= r_sib->entries ? LEFT_SIBLING : RIGHT_SIBLING;
    }
}

static void leaf_shift_from_left(struct bplus_leaf *leaf, struct bplus_leaf *left,
                                 int64_t parent_key_index, int64_t remove)
{
    /* right shift in leaf node */
    for (; remove > 0; remove--) {
        leaf->key[remove] = leaf->key[remove - 1];
//        leaf->data[remove] = leaf->data[remove - 1];
        strcpy(leaf->data[remove],leaf->data[remove - 1]);
    }
    /* borrow the last element from left sibling */
    leaf->key[0] = left->key[left->entries - 1];
//    leaf->data[0] = left->data[left->entries - 1];
    strcpy(leaf->data[0] ,left->data[left->entries - 1]);
    left->entries--;
    /* update parent key */
    leaf->parent->key[parent_key_index] = leaf->key[0];
}

static void leaf_merge_int64_to_left(struct bplus_leaf *leaf, struct bplus_leaf *left, int64_t remove)
{
    int64_t i, j;
    /* merge int64_to left sibling */
    for (i = left->entries, j = 0; j < leaf->entries; j++) {
        if (j != remove) {
            left->key[i] = leaf->key[j];
//            left->data[i] = leaf->data[j];
            strcpy(left->data[i],leaf->data[j]);
            i++;
        }
    }
    left->entries = i;
    /* delete merged leaf */
    leaf_delete(leaf);
}

static void leaf_shift_from_right(struct bplus_leaf *leaf, struct bplus_leaf *right, int64_t parent_key_index)
{
    int64_t i;
    /* borrow the first element from right sibling */
    leaf->key[leaf->entries] = right->key[0];
//    leaf->data[leaf->entries] = right->data[0];
    strcpy(leaf->data[leaf->entries],right->data[0]);
    leaf->entries++;
    /* left shift in right sibling */
    for (i = 0; i < right->entries - 1; i++) {
        right->key[i] = right->key[i + 1];
//        right->data[i] = right->data[i + 1];
        strcpy(right->data[i],right->data[i + 1]);
    }
    right->entries--;
    /* update parent key */
    leaf->parent->key[parent_key_index] = right->key[0];
}

static void leaf_merge_from_right(struct bplus_leaf *leaf, struct bplus_leaf *right)
{
    int64_t i, j;
    /* merge from right sibling */
    for (i = leaf->entries, j = 0; j < right->entries; i++, j++) {
        leaf->key[i] = right->key[j];
//        leaf->data[i] = right->data[j];
        strcpy(leaf->data[i],right->data[j]);
    }
    leaf->entries = i;
    /* delete right sibling */
    leaf_delete(right);
}

static void leaf_simple_remove(struct bplus_leaf *leaf, int64_t remove)
{
    for (; remove < leaf->entries - 1; remove++) {
        leaf->key[remove] = leaf->key[remove + 1];
//        leaf->data[remove] = leaf->data[remove + 1];
        strcpy(leaf->data[remove],leaf->data[remove + 1]);
    }
    leaf->entries--;
}

static int64_t leaf_remove(struct bplus_tree *tree, struct bplus_leaf *leaf, key_bt key)
{
    int64_t remove = key_binary_search(leaf->key, leaf->entries, key);
    if (remove < 0) {
        /* Not exist */
        return -1;
    }

    if (leaf->entries <= (tree->entries + 1) / 2) {
        struct bplus_non_leaf *parent = leaf->parent;
        struct bplus_leaf *l_sib = list_prev_entry(leaf, link);
        struct bplus_leaf *r_sib = list_next_entry(leaf, link);
        if (parent != NULL) {
            /* decide which sibling to be borrowed from */
            int64_t i = leaf->parent_key_idx;
            if (leaf_sibling_select(l_sib, r_sib, parent, i) == LEFT_SIBLING) {
                if (l_sib->entries > (tree->entries + 1) / 2) {
                    leaf_shift_from_left(leaf, l_sib, i, remove);
                } else {
                    leaf_merge_int64_to_left(leaf, l_sib, remove);
                    /* trace upwards */
                    non_leaf_remove(tree, parent, i);
                }
            } else {
                /* remove first in case of overflow during merging with sibling */
                leaf_simple_remove(leaf, remove);
                if (r_sib->entries > (tree->entries + 1) / 2) {
                    leaf_shift_from_right(leaf, r_sib, i + 1);
                } else {
                    leaf_merge_from_right(leaf, r_sib);
                    /* trace upwards */
                    non_leaf_remove(tree, parent, i + 1);
                }
            }
        } else {
            if (leaf->entries == 1) {
                /* delete the only last node */
                assert(key == leaf->key[0]);
                tree->root = NULL;
                leaf_delete(leaf);
                return 0;
            } else {
                leaf_simple_remove(leaf, remove);
            }
        }
    } else {
        leaf_simple_remove(leaf, remove);
    }

    return 0;
}

static int64_t bplus_tree_delete(struct bplus_tree *tree, key_bt key)
{
    struct bplus_node *node = tree->root;
    while (node != NULL) {
        if (is_leaf(node)) {
            struct bplus_leaf *ln = (struct bplus_leaf *)node;
            return leaf_remove(tree, ln, key);
        } else {
            struct bplus_non_leaf *nln = (struct bplus_non_leaf *)node;
            int64_t i = key_binary_search(nln->key, nln->children - 1, key);
            if (i >= 0) {
                node = nln->sub_ptr[i + 1];
            } else {
                i = -i - 1;
                node = nln->sub_ptr[i];
            }
        }
    }
    return -1;
}

char* bplus_tree_get(struct bplus_tree *tree, key_bt key)
{
    //char* data = bplus_tree_search(tree, key);
    //char data[32] ="";
    char *data = (char*)malloc(32);
    strcpy(data,bplus_tree_search(tree, key));
    if (strcmp(data,"0")>0||strcmp(data,"0")<0) {
        return data;
    } else {
        return "-1";
    }
}

//add ICDE test
//int64_t bplus_tree_get_vo(struct bplus_tree *tree, key_bt key, verification_object &vo)
//{
//    int64_t data = bplus_tree_search_vo(tree, key, vo);
//    if (data) {
//        return data;
//    } else {
//        return -1;
//    }
//}

//add e

int64_t bplus_tree_put(struct bplus_tree *tree, key_bt key, char* data, ItemPointer tid)
{
    if ( strcmp(data, "0")>0||strcmp(data, "0")<0) {
        printf("%s%s\n","data is : ",data);
        return bplus_tree_insert(tree, key, data, tid);
    } else {
        printf("%s%d\n","level of tree is: ",tree->level);
        return bplus_tree_delete(tree, key);
    }
}

struct bplus_tree *bplus_tree_init(int64_t order, int64_t entries)
{
    /* The max order of non leaf nodes must be more than two */
    assert(BPLUS_MAX_ORDER > BPLUS_MIN_ORDER);
    assert(order <= BPLUS_MAX_ORDER && entries <= BPLUS_MAX_ENTRIES);

    int64_t i;
    struct bplus_tree *tree = (bplus_tree*)malloc(sizeof(bplus_tree));
//    new bplus_tree;
    if (tree != NULL) {
        tree->root = NULL;
        tree->order = order;
        tree->entries = entries;
        tree->level = 0;
        for (i = 0; i < BPLUS_MAX_LEVEL; i++) {
            list_init(&tree->list[i]);
        }
    }

    return tree;
}

void bplus_tree_deinit(struct bplus_tree *tree)
{
    free(tree);
}

//int64_t bplus_tree_get_range(struct bplus_tree *tree, key_bt key1, key_bt key2)
//{
//    int64_t i, data = 0;
//    key_bt min = key1 <= key2 ? key1 : key2;
//    key_bt max = min == key1 ? key2 : key1;
//    struct bplus_node *node = tree->root;

//    while (node != NULL) {
//            if (is_leaf(node)) {
//                    struct bplus_leaf *ln = (struct bplus_leaf *)node;
//                    i = key_binary_search(ln->key, ln->entries, min);
//                    if (i < 0) {
//                            i = -i - 1;
//                            if (i >= ln->entries) {
//                                    ln = list_next_entry(ln, link);
//                            }
//                    }
//                    while (ln != NULL && ln->key[i] <= max) {
//                            data = ln->data[i];
//                            if (++i >= ln->entries) {
//                                    ln = list_next_entry(ln, link);
//                                    i = 0;
//                            }
//                    }
//                    break;
//            } else {
//                    struct bplus_non_leaf *nln = (struct bplus_non_leaf *)node;
//                    i = key_binary_search(nln->key, nln->children - 1, min);
//                    if (i >= 0) {
//                            node = nln->sub_ptr[i + 1];
//                    } else  {
//                            i = -i - 1;
//                            node = nln->sub_ptr[i];
//                    }
//            }
//    }

//    return data;
//}

#ifdef _BPLUS_TREE_DEBUG
struct node_backlog {
    /* Node backlogged */
    struct bplus_node *node;
    /* The index next to the backtrack point64_t, must be >= 1 */
    int64_t next_sub_idx;
};

static inline int64_t children(struct bplus_node *node)
{
    return ((struct bplus_non_leaf *) node)->children;
}

static void node_key_dump(struct bplus_node *node)
{
    int64_t i;
    if (is_leaf(node)) {
        for (i = 0; i < node->count; i++) {
            printf("%d ", ((struct bplus_leaf *)node)->key[i]);
        }
    } else {
        for (i = 0; i < node->count - 1; i++) {
            printf("%d ", ((struct bplus_non_leaf *)node)->key[i]);
        }
    }
    printf("\n");
}

static key_bt node_key(struct bplus_node *node, int64_t i)
{
    if (is_leaf(node)) {
        return ((struct bplus_leaf *)node)->key[i];
    } else {
        return ((struct bplus_non_leaf *)node)->key[i];
    }
}

static void key_print64_t(struct bplus_node *node)
{
    int64_t i;
    if (is_leaf(node)) {
        struct bplus_leaf *leaf = (struct bplus_leaf *)node;
        printf("leaf:");
        for (i = 0; i < leaf->entries; i++) {
            printf(" %d", leaf->key[i]);
        }
        printf(" hash:");
        for (i = 0; i < leaf->entries; i++) {
            printf(" leaf: %d, hash: %s", i,leaf->childhash[i]);
        }
        printf(" %s", leaf->node_hash);
    } else {
        struct bplus_non_leaf *non_leaf = (struct bplus_non_leaf *)node;
        printf("node:");
        for (i = 0; i < non_leaf->children - 1; i++) {
            printf(" %d", non_leaf->key[i]);
        }
        printf(" hash:");
        //                for (i = 0; i < non_leaf->children; i++) {
        //                        printf(" %s", non_leaf->childhash[i].c_str());
        //                }
        printf(" %s", non_leaf->node_hash);
    }
    printf("\n");
}

void set_node_id(struct bplus_tree *tree,Hashmap* hashp[], int64_t temp_node_id[][500])
{
    //printf("%s%d\n","level of tree is: ",tree->level);

    //first step: attach node id to each node in the tree.
    int i = 0;
    for(i=tree->level;i>=0;i--)
    {
        int id=0;
        if(i==0)
        {
            bplus_leaf* leaf = (bplus_leaf*)((char*)(tree->list[i].next)-(size_t)(&((bplus_leaf *)0)->link));
            while (leaf != NULL)
            {
                leaf->node_id = id;

                //step 2:update parent children offset;
                if(leaf->parent != NULL)
                {
                    leaf->parent_id = leaf->parent->node_id;
                }
                if(leaf->link.next == &(tree->list[i]))
                {
                    break;
                }
                else
                {
                    leaf = list_next_entry(leaf, link);
                    id++;
                }
            }
        }
        else
        {
            bplus_non_leaf* non_leaf = (bplus_non_leaf*)((char*)(tree->list[i].next)-(size_t)(&((bplus_leaf *)0)->link));
            while (non_leaf != NULL)
            {
                non_leaf->node_id = id;
                //step 2:update parent children offset;

                if(non_leaf->parent != NULL)
                {
                    non_leaf->parent_id = non_leaf->parent->node_id;
                }

                if(non_leaf->link.next == &(tree->list[i]))
                {
                    break;
                }
                else
                {
                    non_leaf = list_next_entry(non_leaf, link);
                    id++;
                }
            }
        }
    }

    //second step: store (parent id,children id) map.

//    children_id.clear();
//    vector_clear(&children_id);
    for(i=1;i<=tree->level;i++)
    {
        int id=0;
        bplus_non_leaf* non_leaf =(bplus_non_leaf*)((char*)(tree->list[i].next)-(size_t)(&((bplus_leaf *)0)->link));

        while(true)
        {
            pgvector* children_id = (pgvector*)malloc(sizeof(pgvector));
            vector_setup(children_id,100 ,sizeof(int64_t));
            if(i>1)
            {
                int j = 0;
                int tmp_len = 0;
/*                int uuuu = 0;
                while(uuuu < BPLUS_MAX_ORDER){
                    if(non_leaf->key[uuuu] != 0){
                        tmp_len++;
                        uuuu++;
                    }else{
                        break;
                    }
                }*/
                for(j=0;j<non_leaf->children;j++)
                {
                    int64_t value1 = ((bplus_non_leaf*)non_leaf->sub_ptr[j])->node_id;
                    vector_push_back(children_id, &value1);
                     int x1 = *(int64_t*)vector_get(children_id, j);
                     //printf("%d",x1);
//                    children_id.push_back(((bplus_non_leaf*)non_leaf->sub_ptr[j])->node_id);
                    // cout<<"add to hash table"<<i<<"     hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh    "<<j<<endl;
                }
                // cout<<"children_id: "<<children_id.size()<<endl;
                // cout<<"add to hash table"<<i<<"   aaaa  hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh    "<<non_leaf->node_id<<endl;
//                hashp[i].insert(make_pair(non_leaf->node_id,children_id));
                hashmap_set(hashp[i], non_leaf->node_id, children_id);
                temp_node_id[i][non_leaf->node_id] = 1;
                id++;

                if(non_leaf->link.next == &(tree->list[i]))
                {
                    break;
                }
                else
                {
                    non_leaf = list_next_entry(non_leaf, link);

                }

            }
            else if(i==1)
            {
                int j = 0;
                for(j=0;j<non_leaf->children;j++)
                {
                    int64_t value2 = ((bplus_leaf*)non_leaf->sub_ptr[j])->node_id;
                    vector_push_back(children_id, &value2);
                    int x1 = *(int64_t*)vector_get(children_id, j);
//                     printf("%d",x1);
//                    children_id.push_back(((bplus_leaf*)non_leaf->sub_ptr[j])->node_id);
                }

                hashmap_set(hashp[i], non_leaf->node_id, children_id);
                temp_node_id[i][non_leaf->node_id] = 1;
//                hashp[i].insert(make_pair(non_leaf->node_id,children_id));
                id++;
                 if(non_leaf->node_id == 0){
                    int64_t ids = 0;
                    assert(ids == non_leaf->node_id);
                  pgvector pv = *(struct pgvector *)hashmap_get(hashp[1], ids);
                   printf("test\n");
                }
//                int u= 0;
//                for(u = 0; u< children_id.size; u++){
//                    int zz = *(int*)vector_get(&children_id, u);
//                    printf("%d",zz);
//                }
//                printf("test");
//                             pgvector pv = *(struct pgvector *)hashmap_get(hashp[1], 0);

                if(non_leaf->link.next == &(tree->list[i]))
                {
                    break;
                }
                else
                {
                    non_leaf = list_next_entry(non_leaf, link);
                }
            }
        }
    }


//  pgvector pv = *(struct pgvector *)hashmap_get(hashp[1], 0);
//   printf("test\n");


    //    for(int i =0;i<hashp[tree->level][0].size();i++)
    //    {
    //       cout<<"children: "<<hashp[tree->level][0].size()<<endl;
    //    }


}

int64_t bplus_tree_store(struct bplus_tree *tree)
{
    int64_t root_offset=0;
    char buffer[PAGESIZE];
    memset(buffer, 0, PAGESIZE);
    int i = 0;
    for(i=0;i<=tree->level;i++)
    {
        if(i==tree->level)
        {
            root_offset=tree->offset;
        }
        int id=0;
        // leaf node
        if(i== 0)
        {
            bplus_leaf* node = (bplus_leaf*)((char*)(tree->list[i].next)-(size_t)(&((bplus_leaf *)0)->link));

            //cout<<tree->root<<endl;
            //cout<<node<<endl;
//            if((bplus_node*)node == tree->root)
//            {
//                cout<<"level: "<<i<<" is root"<<endl;
//            }
            int child_count = 0;
            if(node->parent != NULL)
            {

                child_count = node->parent->children;
            }

            int children_number=0;
            while (node != NULL)
            {

                //step 1: write into a file
//                node->serialize(buffer);
                bplus_leaf_serialize(node, buffer);
                //node->print();
                int64_t size= pwrite(tree->fd,buffer,PAGESIZE,tree->offset);

                //step 2:update parent children offset;
                if(node->parent != NULL)
                {
                    node->parent->chindrenoffset[children_number]=tree->offset;
                }
                children_number++;
                tree->offset+=PAGESIZE;
                if(node->link.next == &(tree->list[i]))
                {
                    break;
                }
                else
                {
                    node = list_next_entry(node, link);
                }

                //step 3:change to next parent
                if(children_number>=child_count)
                {
                    if( node->parent !=NULL)
                    {
                        child_count = node->parent->children;
                    }
                    children_number=0;
                }
            }
        }
            //non leaf node
        else
        {

            bplus_non_leaf* non_leaf = (bplus_non_leaf*)((char*)(tree->list[i].next)-(size_t)(&((bplus_leaf *)0)->link));

            int child_count=0;

            if(non_leaf->parent != NULL)
            {
                //cout<<non_leaf->parent<<endl;
                child_count = non_leaf->parent->children;
            }

            int children_number=0;

            while (non_leaf != NULL)
            {
                //step 1: write into a file
                bplus_non_leaf_serialize(non_leaf, buffer);
                int64_t size = pwrite(tree->fd,buffer,PAGESIZE,tree->offset);


                //step 2:update parent children offset;
                if(non_leaf->parent != NULL)
                {
                    non_leaf->parent->chindrenoffset[children_number]=tree->offset;
                }
                children_number++;

                tree->offset+=PAGESIZE;

                if(non_leaf->link.next == &(tree->list[i]))
                {
                    break;
                }
                else
                {
                    non_leaf = list_next_entry(non_leaf, link);
                }

                //step 3:change to next parent
                if(children_number==child_count)
                {

                    if(non_leaf->parent != NULL)
                    {
                        child_count = non_leaf->parent->children;
                    }
                    children_number=0;
                }
            }
        }
    }
    return root_offset;
}

/*
int64_t bplus_tree_get_range_disk(struct bplus_tree *tree, key_bt key1, key_bt key2,
                                  vector<int64_t> *result,int64_t root_offset)
{
    int ret =0;
    int64_t fd= open("/home/zcy/opt/treedata/treedata",O_RDWR);
    char buf[4096];
    pread(fd,buf,PAGESIZE,root_offset);
    bplus_non_leaf *nonleaf = (bplus_non_leaf*)malloc(sizeof(bplus_non_leaf));
            //new bplus_non_leaf();
//    nonleaf->deserialize(buf);
    bplus_non_leaf_deserialize(nonleaf, buf);
//    nonleaf->print();
    int64_t level= tree->level;

    int64_t i, data = 0;
    key_bt min = key1 <= key2 ? key1 : key2;
    key_bt max = min == key1 ? key2 : key1;
    int64_t read_offset=0;

    //search nonleaf node
    i = key_binary_search(nonleaf->key, nonleaf->children - 1, min);
    bplus_non_leaf* tmp =NULL;
    tmp = nonleaf;

    //    cout<<"root================================================="<<endl;
    //    nonleaf->print();


    bplus_non_leaf* non_leaf1 = (bplus_non_leaf*)malloc(sizeof(bplus_non_leaf));
            //new bplus_non_leaf();
    level --;
    while(level>0)
    {
        char *tmp_buf = (char*)malloc(PAGESIZE);
        if (i >= 0) {
            //cout<<"i>0 ================================================="<<endl;
            read_offset =tmp->chindrenoffset[i+1];
            pread(fd,tmp_buf,PAGESIZE,read_offset);
//            non_leaf1->deserialize(tmp_buf);
            bplus_non_leaf_deserialize(non_leaf1, tmp_buf);
            //non_leaf1->print();
        } else  {
            //cout<<"i<0 ================================================="<<endl;
            i = -i - 1;
            read_offset =tmp->chindrenoffset[i];
            pread(fd,tmp_buf,PAGESIZE,read_offset);
//            non_leaf1->deserialize(tmp_buf);
             bplus_non_leaf_deserialize(non_leaf1, tmp_buf);
            //non_leaf1->print();
        }
        i = key_binary_search(non_leaf1->key, non_leaf1->children - 1, min);
        tmp = non_leaf1;
        level --;
    }

    //search leaf node, level =0
    char *leaf_tmp_buf = (char*)malloc(PAGESIZE);
    struct bplus_leaf *ln = (struct bplus_leaf *)malloc(sizeof(bplus_leaf));
            //new bplus_leaf();
    int64_t leaf_read_offset =0;
    if (i >= 0)
    {
        leaf_read_offset =tmp->chindrenoffset[i+1];
        pread(fd,leaf_tmp_buf,PAGESIZE,leaf_read_offset);
//        ln->deserialize(leaf_tmp_buf);
         bplus_leaf_deserialize(ln, leaf_tmp_buf);
    }

    else
    {
        i=-i-1;
        leaf_read_offset =tmp->chindrenoffset[i];
        pread(fd,leaf_tmp_buf,PAGESIZE,leaf_read_offset);
//        ln->deserialize(leaf_tmp_buf);
        bplus_leaf_deserialize(ln, leaf_tmp_buf);
    }

    //ln->print();

    i = key_binary_search(ln->key, ln->entries, min);


    if (i < 0)
    {
        i = -i - 1;
        if (i >= ln->entries)
        {
            leaf_read_offset+=PAGESIZE;
            pread(fd,leaf_tmp_buf,PAGESIZE,leaf_read_offset);
//            ln->deserialize(leaf_tmp_buf);
            bplus_leaf_deserialize(ln, leaf_tmp_buf);
        }
    }
    while (ln != NULL && ln->key[i] <= max)
    {
        //cout<<"test"<<ln->key[i]<<endl;
        data = ln->data[i];
        result->push_back(data);
        if (++i >= ln->entries)
        {
            leaf_read_offset+=PAGESIZE;
            pread(fd,leaf_tmp_buf,PAGESIZE,leaf_read_offset);
//            ln->deserialize(leaf_tmp_buf);
            bplus_leaf_deserialize(ln, leaf_tmp_buf);
            i=0;
        }
    }
}
*/
bplus_non_leaf* tmp10 = NULL;
int64_t bplus_tree_get_range_disk_vo(struct bplus_tree *tree, key_bt key1, key_bt key2,int64_t root_offset,
                                    struct pgvector *result,rangevo *vo, struct pgvector *keys_result)
{
//    int ret =0;
    //get the root node
//    int64_t fd= open("/home/jcq/zcy/opt/treedata/treedata",O_RDWR);
//    char buf[PAGESIZE];
//    memset(buf, 0, PAGESIZE);
//    pread(fd,buf,PAGESIZE,root_offset);
//    struct bplus_non_leaf *nonleaf = (struct bplus_non_leaf *)malloc(sizeof(bplus_non_leaf));
//    bplus_non_leaf_init(nonleaf);
//    bplus_non_leaf_deserialize(nonleaf, buf);
//    int64_t level= tree->level;
    struct bplus_non_leaf *nonleaf =  (struct  bplus_non_leaf*)tree->root;
    int64_t level= tree->level;

    int64_t i = 0;
//    char* data = (char*)malloc(33);
//    strcpy(data,"0");
    key_bt min = key1 <= key2 ? key1 : key2;
    key_bt max = min == key1 ? key2 : key1;
//    int64_t read_offset=0;
//    startTime = GetTime();
    //first step: get left search path
    //printf("----------------------------------------------------------------------begin step 1: right left path----------------------------------------------------------------------\n");
    bplus_non_leaf* tmp =NULL;
    bplus_leaf* tmp1 = NULL;
    tmp = nonleaf;

    char* tmp_buf = (char*)malloc(PAGESIZE);
    memset(tmp_buf, 0, PAGESIZE);
    bplus_non_leaf* non_leaf1 = NULL;
    bplus_leaf* leaf1 = NULL;
    while(level>0)
    {
//        bplus_non_leaf* non_leaf1 = (struct bplus_non_leaf *)malloc(sizeof(bplus_non_leaf));
//        bplus_non_leaf_init(non_leaf1);
                /** new bplus_non_leaf();*/
//        bplus_leaf* leaf1 = (struct bplus_leaf *)malloc(sizeof(bplus_leaf));
//        bplus_leaf_init(leaf1);
//                new bplus_leaf();
        //cout<<"level "<<level<< "------------------------------------------------"<<endl;
        int tmp_len = 0;
        int uuuu = 0;
        while(uuuu < BPLUS_MAX_ORDER){
            if(tmp->key[uuuu] != 0){
                tmp_len++;
                uuuu++;
            }else{
                break;
            }
        }

        i = key_binary_search(tmp->key, tmp_len, min);
        if (i >= 0) {
            vo->leftvo[vo->traverse_height].node_id = tmp->node_id;
            vo->leftvo[vo->traverse_height].bound_node_childno = tmp->children;
            int p = 0;
            for(p=0;p<=i;p++)
            {
                char* temp_hash = (char*)malloc(33);
                strcpy(temp_hash, tmp->childhash[p]);
                vector_push_back(&vo->leftvo[vo->traverse_height].nhash, temp_hash);
            }
//            read_offset =tmp->chindrenoffset[i+1];
//            pread(fd,tmp_buf,PAGESIZE,read_offset);
//            if(level!=1)
//            {
//                bplus_non_leaf_init(non_leaf1);
//                bplus_non_leaf_deserialize(non_leaf1, tmp_buf);
//                 memset(tmp_buf, 0, PAGESIZE);
//            }
//            else
//            {
//                bplus_leaf_init(leaf1);
//                bplus_leaf_deserialize(leaf1, tmp_buf);
//                 memset(tmp_buf, 0, PAGESIZE);
//            }
            if(level!=1)
              {
                   non_leaf1 = (bplus_non_leaf*)tmp->sub_ptr[i+1];
              }
              else
              {
                   leaf1 = (bplus_leaf*)(tmp->sub_ptr[i+1]);
              }
        } else  {
            i = -i - 1;
            vo->leftvo[vo->traverse_height].node_id = tmp->node_id;
            vo->leftvo[vo->traverse_height].bound_node_childno = tmp->children;
            int p =0;
            for(p=0;p<i;p++)
            {
                char* temp_hash = (char*)malloc(33);
                strcpy(temp_hash, tmp->childhash[p]);
                vector_push_back(&vo->leftvo[vo->traverse_height].nhash, temp_hash);
            }
//            read_offset =tmp->chindrenoffset[i];
//            pread(fd,tmp_buf,PAGESIZE,read_offset);
//            if(level!=1)
//            {
//                bplus_non_leaf_init(non_leaf1);
//                bplus_non_leaf_deserialize(non_leaf1,tmp_buf);
//                 memset(tmp_buf, 0, PAGESIZE);
//            }
//            else
//            {
//                bplus_leaf_init(leaf1);
//                bplus_leaf_deserialize(leaf1, tmp_buf);
//                 memset(tmp_buf, 0, PAGESIZE);
//            }
            if(level!=1)
            {
                 non_leaf1 = (bplus_non_leaf*)tmp->sub_ptr[i];
            }
            else
            {
                leaf1 = (bplus_leaf*)tmp->sub_ptr[i];
            }
        }
        tmp = non_leaf1;
        tmp1 = leaf1;
        vo->traverse_height++;
        level --;
    }

    //printf("----------------------------------------------------------------------begin step 2: leaf node scan----------------------------------------------------------------------\n");
    //search leaf node, level =0

//    char *leaf_tmp_buf = (char*)malloc(PAGESIZE);
//    memset(leaf_tmp_buf,0,PAGESIZE);
    struct bplus_leaf *ln = (struct bplus_leaf *)malloc(sizeof(bplus_leaf));
//    int64_t leaf_read_offset =0;
    i = key_binary_search(tmp1->key, tmp1->entries, min);
    if (i >= 0)
    {
        vo->leftvo[vo->traverse_height].node_id = tmp1->node_id;
        vo->leftvo[vo->traverse_height].bound_node_childno = tmp1->entries;
        int p = 0;
        for(p=0;p<i;p++)
        {
            char* temp_hash = (char*)malloc(33);
            strcpy(temp_hash, tmp1->childhash[p]);
            vector_push_back(&vo->leftvo[vo->traverse_height].nhash, temp_hash);
        }
    }
    else
    {
        i=-i-1;
        vo->leftvo[vo->traverse_height].node_id = tmp1->node_id;
        vo->leftvo[vo->traverse_height].bound_node_childno = tmp1->entries;
        int p = 0;
        for(p=0;p<i;p++)
        {
            char* temp_hash = (char*)malloc(33);
            strcpy(temp_hash, tmp1->childhash[p]);
            vector_push_back(&vo->leftvo[vo->traverse_height].nhash, temp_hash);
        }
    }
//    endTime = GetTime();
//    mytime =mytime + ((double)(endTime - startTime)) ;
//    count1++;
//    if(count1==99){
//       printf("\n\n\n\nauthrangequeryselect times = %f\n\n\n\n",mytime);
//       count1=0;
//       mytime=0;
//    }
//    startTime = GetTime();
    ln=tmp1;
    i = key_binary_search(ln->key, ln->entries, min);
    if (i < 0)
    {
        i = -i - 1;
        if (i >= ln->entries)
        {
//            leaf_read_offset+=PAGESIZE;
//            pread(fd,leaf_tmp_buf,PAGESIZE,leaf_read_offset);
//            bplus_leaf_deserialize(ln, leaf_tmp_buf);
//            i=0;
            ln = list_next_entry(ln, link);
            i=0;
        }
    }
    pgvector nodedata;
    vector_setup(&nodedata, 60, sizeof(pgvector));
    while (ln != NULL && ln->key[i] <= max)
    {
        char* data1 = (char*)malloc(33);
        strcpy(data1,ln->data[i]);
        vector_push_back(&nodedata, &data1);
//        startTime =GetTime();
        if(ln->key[i]  >= key1 && ln->key[i]<= key2){
            ItemPointer tid = (ItemPointer)malloc(sizeof(ItemPointerData));
            tid->ip_blkid.bi_hi = ln->tids[i]->ip_blkid.bi_hi;
            tid->ip_blkid.bi_lo = ln->tids[i]->ip_blkid.bi_lo;
            tid->ip_posid = ln->tids[i]->ip_posid;
            vector_push_back(result, &tid);
            int64_t mykey = ln->key[i];
            vector_push_back(keys_result, &mykey);
        }
//        endTime = GetTime();
//        mytime =mytime + ((double)(endTime - startTime)) ;
//        startTime = GetTime();
        if (++i >= ln->entries)
        {
            vector_push_back(&vo->data, &nodedata);
            vector_setup(&nodedata, 60, sizeof(pgvector));
//            leaf_read_offset+=PAGESIZE;
//            memset(leaf_tmp_buf,0,PAGESIZE);
//            pread(fd,leaf_tmp_buf,PAGESIZE,leaf_read_offset);
//            bplus_leaf_deserialize(ln, leaf_tmp_buf);
//            i=0;
            ln = list_next_entry(ln, link);
            i=0;
        }
//        endTime = GetTime();
//        mytime =mytime + ((double)(endTime - startTime)) ;
    }
    //add last node
    vector_push_back(&vo->data, &nodedata);
//    endTime = GetTime();
//    mytime =mytime + ((double)(endTime - startTime)) ;
//    count1++;
//    if(count1==99){
//       printf("\n\n\n\nauthrangequeryselect times = %f\n\n\n\n",mytime);
//       count1=0;
//       mytime=0;
//    }
    //printf("----------------------------------------------------------------------begin step 3: right search path----------------------------------------------------------------------\n");
    //step 3: right search path
    //startTime = GetTime();
    int64_t elevel= tree->level;
    vo->traverse_height = 0;
    int64_t ei = 0;
    bplus_non_leaf* tmp2 =NULL;
    bplus_leaf* tmp3 =NULL;
    tmp2 = nonleaf;

//    char *tmp_buf1 = (char*)malloc(PAGESIZE);
//    memset(tmp_buf1, 0, PAGESIZE);
    bplus_non_leaf* non_leaf2 = NULL;
    bplus_leaf* leaf2 = NULL;
    while(elevel>0)
    {
//        bplus_non_leaf* non_leaf2 = (struct bplus_non_leaf *)malloc(sizeof(bplus_non_leaf));


//                new bplus_non_leaf();
//        bplus_leaf* leaf2 = (struct bplus_leaf *)malloc(sizeof(bplus_leaf));
//                new bplus_leaf();
        int tmp_len = 0;
        int uuuu = 0;
        while(uuuu < BPLUS_MAX_ORDER){
            if(tmp2->key[uuuu] != 0){
                tmp_len++;
                uuuu++;
            }else{
                break;
            }
        }
        ei = key_binary_search(tmp2->key, tmp_len, max);
        if (ei >= 0) {
            vo->rightvo[vo->traverse_height].node_id = tmp2->node_id;
            vo->rightvo[vo->traverse_height].bound_node_childno = tmp2->children;
            int p = 0;
            for(p=ei+2;p<tmp2->children;p++)
            {
                if(vo->traverse_height == 1){
                }
                vector_push_back(&vo->rightvo[vo->traverse_height].nhash, tmp2->childhash[p]);
            }
//            read_offset =tmp2->chindrenoffset[ei+1];
//            pread(fd,tmp_buf1,PAGESIZE,read_offset);
//            if(elevel !=1)
//            {
//                bplus_non_leaf_init(non_leaf2);
//                bplus_non_leaf_deserialize(non_leaf2, tmp_buf1);
//                 memset(tmp_buf1, 0, PAGESIZE);
//            }
//            else
//            {
//                bplus_leaf_init(leaf2);
//                bplus_leaf_deserialize(leaf2, tmp_buf1);
//                 memset(tmp_buf1, 0, PAGESIZE);
//            }
            if(elevel !=1)
             {
                 non_leaf2 = (bplus_non_leaf*)tmp2->sub_ptr[ei+1];
             }
             else
             {
                 leaf2 = (bplus_leaf*)tmp2->sub_ptr[ei+1];
             }
        } else  {
            ei = -ei - 1;
            vo->rightvo[vo->traverse_height].node_id = tmp2->node_id;
            vo->rightvo[vo->traverse_height].bound_node_childno = tmp2->children;
            int p = 0;
            for(p=ei+1;p<tmp2->children;p++)
            {
                if(vo->traverse_height == 1){
                }
                char* tempc = (char*)malloc(33);
                strncpy(tempc,tmp2->childhash[p],33);
                vector_push_back(&vo->rightvo[vo->traverse_height].nhash, tempc);
            }

//            read_offset =tmp2->chindrenoffset[ei];
//            pread(fd,tmp_buf1,PAGESIZE,read_offset);
//            if(elevel !=1)
//            {
//                bplus_non_leaf_init(non_leaf2);
//                bplus_non_leaf_deserialize(non_leaf2, tmp_buf1);
//            }
//            else
//            {
//                bplus_leaf_init(leaf2);
//                bplus_leaf_deserialize(leaf2, tmp_buf1);
//            }
            if(elevel !=1)
              {
                  non_leaf2 = (bplus_non_leaf*)tmp2->sub_ptr[ei];

              }
              else
              {
                  leaf2 = (bplus_leaf*)tmp2->sub_ptr[ei];
              }
        }
        tmp2 = non_leaf2;
        tmp3 = leaf2;
        vo->traverse_height++;
        elevel --;
    }

    ei = key_binary_search(tmp3->key, tmp3->entries, max);

    if (ei >= 0)
    {
        vo->rightvo[vo->traverse_height].node_id = tmp3->node_id;
        vo->rightvo[vo->traverse_height].bound_node_childno = tmp3->entries;
        int p = 0;
        for( p=ei+1;p<tmp3->entries;p++)
        {
            if(vo->traverse_height == 1){
            }
            vector_push_back(&vo->rightvo[vo->traverse_height].nhash, tmp3->childhash[p]);
        }
    }
    else
    {
        ei=-ei-1;
        vo->rightvo[vo->traverse_height].node_id = tmp3->node_id;
        vo->rightvo[vo->traverse_height].bound_node_childno = tmp3->entries;
        int p =0;
        for(p=ei+1;p<tmp3->entries;p++)
        {
            if(vo->traverse_height == 1){
            }
            vector_push_back(&vo->rightvo[vo->traverse_height].nhash, tmp3->childhash[p]);
        }
    }
//    endTime = GetTime();
//    mytime =mytime + ((double)(endTime - startTime)) ;
//    count1++;
//    if(count1==99){
//       printf("\n\n\n\nauthrangequeryselect times = %f\n\n\n\n",mytime);
//       count1=0;
//       mytime=0;
//    }
    if(tmp_buf != NULL){
        free(tmp_buf);
        tmp_buf = NULL;
    }
//    if(nonleaf != NULL){
//        free(non_leaf1);
//        non_leaf1 = NULL;
//    }
//    if(leaf1 != NULL){
//        free(leaf1);
//        leaf1 = NULL;
//    }
//    if(leaf2 != NULL){
//        free(leaf2);
//        leaf2 = NULL;
//    }
//    if(data != NULL){
//        free(data);
//        data = NULL;
//    }
//    if(ln != NULL){
//        free(ln);
//        ln = NULL;
//    }
//    if(tmp_buf1 != NULL){
//        free(tmp_buf1);
//        tmp_buf1 = NULL;
//    }
//    if(non_leaf1 != NULL){
//        free(non_leaf1);
//        non_leaf1 = NULL;
//    }

//    if(leaf2 != NULL){
//        free(leaf2);
//        leaf2 = NULL;
//    }
//    if(non_leaf2 != NULL){
//        free(non_leaf2);
//        non_leaf2 = NULL;
//    }
//    if(leaf_tmp_buf != NULL){
//        free(leaf_tmp_buf);
//        leaf_tmp_buf = NULL;
//    }
//    close(fd);

//    if(ln != NULL){
//        free(ln);
//        ln = NULL;
//    }

//    free(tmp_buf);
//    free(non_leaf1);
//    free(leaf1);
//    free(nonleaf);
//    free(data);
//    free(leaf_tmp_buf);
//    free(ln);
//    free(tmp_buf1);
}

void bplus_tree_dump(struct bplus_tree *tree)
{
    int64_t level = 0;
    struct bplus_node *node = tree->root;
    struct node_backlog *p_nbl = NULL;
    struct node_backlog nbl_stack[BPLUS_MAX_LEVEL];
    struct node_backlog *top = nbl_stack;

    for (; ;) {
        if (node != NULL) {
            /* non-zero needs backward and zero does not */
            int64_t sub_idx = p_nbl != NULL ? p_nbl->next_sub_idx : 0;
            /* Reset each loop */
            p_nbl = NULL;

            /* Backlog the path */
            if (is_leaf(node) || sub_idx + 1 >= children(node)) {
                top->node = NULL;
                top->next_sub_idx = 0;
            } else {
                top->node = node;
                top->next_sub_idx = sub_idx + 1;
            }
            top++;
            level++;

            /* Draw the whole node when the first entry is passed through */
            if (sub_idx == 0) {
                int64_t i;
                for (i = 1; i < level; i++) {
                    if (i == level - 1) {
                        printf("%-8s", "+-------");
                    } else {
                        if (nbl_stack[i - 1].node != NULL) {
                            printf("%-8s", "|");
                        } else {
                            printf("%-8s", " ");
                        }
                    }
                }
                key_print64_t(node);
            }

            /* Move deep down */
            node = is_leaf(node) ? NULL : ((struct bplus_non_leaf *) node)->sub_ptr[sub_idx];
        } else {
            p_nbl = top == nbl_stack ? NULL : --top;
            if (p_nbl == NULL) {
                /* End of traversal */
                break;
            }
            node = p_nbl->node;
            level--;
        }
    }
}


char* reconstruct_root(struct Hashmap* hashp[],rangevo vo, char *root)
{
    memset(root,0,33);
//  unordered_map<int64_t,char*>tmp_hashes[BPLUS_MAX_LEVEL];

    Hashmap* tmp_hashes[BPLUS_MAX_LEVEL];
    int i = 0;
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        tmp_hashes[i] = hashmap_new(NUM_BUCKETS, myhash, compare);
    }

    //for each level
    int level_count = 0;
    //cout<<"begin to reconstruct root"<<endl;
    int mapvalue= vo.traverse_height+1;

    //first construct hash value of leaf nodes
    i=vo.traverse_height;
    int64_t start_id = vo.leftvo[i].node_id;
    int64_t end_id = vo.rightvo[i].node_id;
//    printf("%s%d%s%d%s%d\n","traverse_height: ",i," start_id: ",start_id," end_id: ",end_id);

    //used to calculate which node
    int node_count=0;
    char* hashca = (char*)malloc(8192);
    memset(hashca,0,8192);

    //calculate hash value of left boundary node
    int k = 0;
    for(k=0;k<vo.leftvo[i].nhash.size;k++)
    {
        //cout<<(vo.leftvo[i]).nhash.at(k).c_str()<<endl;
        //hashca+= vo.leftvo[i].nhash.at(k);
        char* temp_hash = (char*)vector_get(&vo.leftvo[i].nhash, k);
        printf("%s%s\n","leftvo.hash: ",temp_hash);
        //strcat(hashca,vo.leftvo[i].nhash.at(k));
        strcat(hashca, temp_hash);
    }

    pgvector myvector1 = *(pgvector*)vector_get(&vo.data, node_count);
    int j=0;
    for(j=0;j<myvector1.size;j++)
    {
        char* tmp_id= *(char**)vector_get(&myvector1, j);
//        printf("%s%s\n","left boundary node: ",tmp_id);
//        char* tmp_id=(char*)malloc(32);
//        memset(tmp_id, 0, 32);
//        char* temp_value = *(char**)vector_get(&myvector1, j);
//        sprintf(tmp_id, "%s", temp_value);
//        printf("%s%d\n","left boundary node: ",tmp_id);

        char* result=(char*)malloc(33);
        memset(result, 0, 33);
        sha256_pg(tmp_id,result);
        strcat(hashca,result);
    }
    char *ttt = (char*)malloc(33);
    memset(ttt, 0, 33);
    sha256_pg(hashca,ttt);
    free(hashca);
//    tmp_hashes[level_count].insert(make_pair(start_id,hashca));
    hashmap_set(tmp_hashes[level_count], start_id, ttt);

    //calculate hash values node of continuous nodes
    node_count++;
//    if (start_id==end_id){
//        node_count=start_id;
//    }else{
//        node_count=start_id+1;
//    }

    int m = 0;
    
    for(m=start_id+1;m<end_id; m++)
    {
        char* mhash = (char*)malloc(8192);
        //cout<<node_count<<endl;
        memset(mhash, 0, 8192);
        pgvector myvector2 = *(pgvector*)vector_get(&vo.data, node_count);
        int j = 0;
        for(j =0 ; j < myvector2.size; j++)
        {
            //cout<<vo.data.at(node_count).at(j)<<endl;
            //char* tmp_id =(char*)vo.data.at(node_count).at(j);

//            char* tmp_id = (char*)malloc(32);
//            memset(tmp_id, 0, 32);
//            char* temp_value =  (char*)vector_get(&myvector2, j);
//            strcpy(tmp_id,  temp_value);
//            printf("%s%d\n","node_count: ",node_count);
//            printf("%s%s\n","continuous nodes: ",tmp_id);

            char* tmp_id = *(char**)vector_get(&myvector2, j);
//            printf("%s%s\n","right boundary node: ",tmp_id);

            char* result=(char*)malloc(33);
            memset(result, 0, 33);
            sha256_pg(tmp_id,result);
            strcat(mhash,result);
            free(result);
        }
        char* tmp_mhash=(char*)malloc(33);
        memset(tmp_mhash, 0, 33);
        sha256_pg(mhash,tmp_mhash);
        free(mhash);
        //cout<<mhash.c_str()<<endl;
//        tmp_hashes[level_count].insert(make_pair(m,mhash));
        hashmap_set(tmp_hashes[level_count], m, tmp_mhash);
        node_count++;
    }

    //calculate hash value of right boundary node
    char* ehash = (char*)malloc(8192);
    memset(ehash, 0, 8192);
    //if(start_id!=0&&end_id!=0&& start_id==end_id &&  vo.rightvo[i].nhash.size!=0 ){
    if(start_id==end_id ){
        pgvector myvector2 = *(pgvector*)vector_get(&vo.data, node_count-1);
        for(j =0 ; j<myvector2.size;j++)
        {
            //cout<<vo.data.at(node_count).at(j)<<endl;
            //char* tmp_id =(char*)Yvo.data.at(node_count).at(j);
            //ehash+= (char*)sha256(tmp_id);
            //strcat( ehash,(char*)sha256(tmp_id));

    //        char* tmp_id = (char*)malloc(100);
    //        char* temp_value = (char*)vector_get(&myvector2, j);
    //        strcpy(tmp_id,  temp_value);
    //        printf("%s%d\n","node_count: ",node_count);
    //        printf("%s%s\n","right boundary node: ",tmp_id);

            char* tmp_id = *(char**)vector_get(&myvector2, j);
//            printf("%s%s\n","right boundary node: ",tmp_id);

            char* result=(char*)malloc(33);
            memset(result, 0, 33);
            sha256_pg(tmp_id,result);
            strcat(ehash,result);
        }
    }else{
        pgvector myvector2 = *(pgvector*)vector_get(&vo.data, node_count);
        for(j =0 ; j<myvector2.size;j++)
        {
            //cout<<vo.data.at(node_count).at(j)<<endl;
            //char* tmp_id =(char*)vo.data.at(node_count).at(j);
            //ehash+= (char*)sha256(tmp_id);
            //strcat( ehash,(char*)sha256(tmp_id));

    //        char* tmp_id = (char*)malloc(100);
    //        char* temp_value = (char*)vector_get(&myvector2, j);
    //        strcpy(tmp_id,  temp_value);
    //        printf("%s%d\n","node_count: ",node_count);
    //        printf("%s%s\n","right boundary node: ",tmp_id);

            char* tmp_id = *(char**)vector_get(&myvector2, j);
            //printf("%s%s\n","right boundary node: ",tmp_id);

            char* result=(char*)malloc(33);
            memset(result, 0, 33);
            sha256_pg(tmp_id,result);
            strcat(ehash,result);
        }
    }


    for(j=0;j< vo.rightvo[i].nhash.size;j++)
    {
        //ehash+= vo.rightvo[i].nhash.at(j);
        char *my_hash = (char*)vector_get(&vo.rightvo[i].nhash, j);
        printf("%s%d\n","node_count: ",node_count);
        printf("%s%s\n","rightvo.hash: ",my_hash);
        strcat(ehash, my_hash);
    }
    //ehash = (char*)sha256(ehash);
    char* tmp_ehash = (char*)malloc(33);
    memset(tmp_ehash, 0, 33);
    sha256_pg(ehash,tmp_ehash);
    free(ehash);
   // cout<<ehash.c_str()<<endl;
//    tmp_hashes[level_count].insert(make_pair(end_id,ehash));
    hashmap_set(tmp_hashes[level_count], end_id, tmp_ehash);

    //for non leaf node
    for(k=1;k<=vo.traverse_height;k++)
    {
        i--;
        start_id = vo.leftvo[i].node_id;
        int64_t end_id = vo.rightvo[i].node_id;
        char* hashcau=(char*)malloc(8192);
        memset(hashcau,0,8192);

        if(start_id !=end_id)
        {
            //calculate hash value of left boundary node
            int j = 0;
            for(j=0;j<vo.leftvo[i].nhash.size;j++)
            {
                char *my_hash = (char*)vector_get(&vo.leftvo[i].nhash, j);
                //hashca+= vo.leftvo[i].nhash.at(j);
                strcat(hashcau, my_hash);
            }
            //get each children of the node
//            pgvector pv111 = *(struct pgvector *)hashmap_get(hashp[1], 0);
            pgvector pv = *(struct pgvector *)hashmap_get(hashp[k], start_id);

//          pgvector pv = hashp[k][start_id];
            int vsize = pv.size;
//            for(int j=0; j< hashp[k][start_id].size();j++)
            for(j=0; j< vsize;j++)
            {
//                int64_t child_id = hashp[k][start_id].at(j);
                  int64_t child_id = *(int64_t*)vector_get(&pv, j);
//                char* tmp_id = (char*)malloc(1000);
//                sprintf(tmp_id, "%d", vo.data.at(node_count).at(j));
                char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], child_id);
                if(ss_temp != NULL){
                    strcat(hashcau, ss_temp /*tmp_hashes[level_count][child_id]*/);
                }
                //hashca+= tmp_hashes[level_count][child_id];

            }

            //calculate hash value of this node
            //hashca = (char*)sha256(hashca);

            char *tmp_hashca = (char*)malloc(33);
            memset(tmp_hashca, 0, 33);
            sha256_pg(hashcau,tmp_hashca);
            free(hashcau);
            hashmap_set(tmp_hashes[level_count+1], start_id, tmp_hashca);
//            tmp_hashes[level_count+1].insert(make_pair(start_id,hashca));

            //calculate hash values node of continuous nodes
            int p = 0;
            for(p=start_id+1;p<end_id;p++)
            {
                char* phash=(char*)malloc(8192 * 3);
                memset(phash,0,8192*3);
                pgvector childs = *(struct pgvector *)hashmap_get(hashp[k], p);
//                pgvector childs = hashp[k][p];
                int i = 0;
                for(i =0;i<childs.size;i++)
                {
                    //phash+=tmp_hashes[level_count][childs[i]];
                    int64_t index = *(int64_t*)vector_get(&childs, i);
                    char* ss_temp = (char*)hashmap_get(tmp_hashes[level_count], index);
                    if(ss_temp != NULL){
                        strcat(phash,  ss_temp/*tmp_hashes[level_count][index]*/);
                        free(ss_temp);
                    }
                }
                //phash = (char*)sha256(phash);
                char *tmp_phash = (char*)malloc(33);
                memset(tmp_phash, 0, 33);
                sha256_pg(phash,tmp_phash);
                free(phash);
                hashmap_set(tmp_hashes[level_count+1], p, tmp_phash);
//                tmp_hashes[level_count+1].insert(make_pair(p,phash));
            }


            //calculate hash value of right boundary node
            char* ehash2=(char*)malloc(8192);
            memset(ehash2, 0, 8192);
            pgvector pv1 = *(struct pgvector *)hashmap_get(hashp[k], end_id);
//            pgvector pv1 = hashp[k][end_id];
            for(j =0 ; j < pv1.size;j++)
            {
//                int64_t child_id = hashp[k][end_id].at(j);
                int64_t child_id = *(int64_t*)vector_get(&pv1, j);
                //ehash+= tmp_hashes[level_count][child_id];
                char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], child_id);
                if(ss_temp != NULL){
                    strcat(ehash2,ss_temp);
                    free(ss_temp);
                }
            }

            for(j=0;j<vo.rightvo[i].nhash.size;j++)
            {
                //ehash+= vo.rightvo[i].nhash.at(j);
                 char *my_hash = (char*)vector_get(&vo.rightvo[i].nhash, j);
                 strcat(ehash2, my_hash);
            }
            //ehash = (char*)sha256(ehash);
            char * tmp_ehash2 = (char*)malloc(33);
            memset(tmp_ehash2, 0, 33);
            sha256_pg(ehash2,tmp_ehash2);
            free(ehash2);
            hashmap_set(tmp_hashes[level_count+1], end_id, tmp_ehash2);
//            tmp_hashes[level_count+1].insert(make_pair(end_id,ehash));
            level_count++;
        }
        else
        {
            int j = 0;
            //calculate hash value of left boundary node
            for(j=0;j<vo.leftvo[i].nhash.size;j++)
            {
                 char *my_hash = (char*)vector_get(&vo.leftvo[i].nhash, j);
                //hashca+= vo.leftvo[i].nhash.at(j);
                strcat(hashcau,my_hash);
            }
            //get each children of the node
            pgvector pv2 = *(struct pgvector *)hashmap_get(hashp[k], start_id);
//            pgvector pv2 = hashp[k][start_id];
            for(j=0;j<pv2.size;j++)
            {
//                int64_t child_id = hashp[k][start_id].at(j);
                int64_t child_id = *(int64_t*)vector_get(&pv2, j);
                //hashca+= tmp_hashes[level_count][child_id];
                char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], child_id);
                if(ss_temp != NULL){
                     strcat(hashcau,ss_temp);
                     free(ss_temp);
                }
            }

            for(j=0;j<vo.rightvo[i].nhash.size;j++)
            {
                //hashca+= vo.rightvo[i].nhash.at(j);
                char *my_hash = (char*)vector_get(&vo.rightvo[i].nhash, j);
                strcat(hashcau, my_hash);
            }
            //hashca = (char*)sha256(hashca);
            char *tmp_hashca = (char*)malloc(33);
            memset(tmp_hashca, 0, 33);
            sha256_pg(hashcau, tmp_hashca);
            free(hashcau);
            hashmap_set(tmp_hashes[level_count+1], end_id, tmp_hashca);
//            tmp_hashes[level_count+1].insert(make_pair(end_id,hashca));
            level_count++;
        }
    }
    char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], 0);
    strcpy(root , ss_temp);
    free(ss_temp);
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        struct Hashmap* hp = tmp_hashes[i];
        hashmap_free(hp);
        hp = NULL;
    }
    return root;
}

void hashmap_serialize(Hashmap* hashp[BPLUS_MAX_LEVEL], int64_t temp_node_id[10][500],  char* buffer){
      int i = 0, j = 0, k = 0;
    int64_t pos = 0;
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        struct Hashmap* hp = hashp[i];
        int64_t hp_size = hp->size;
        memcpy(buffer+pos, &hp_size, sizeof(int64_t));
        pos+=sizeof(int64_t);
        if(hp->size > 0){
            for(j = 0; j < 500; j++){
                int64_t xx = temp_node_id[i][j];
                if(temp_node_id[i][j] == 0)
                    continue;
                int64_t index = j;
                memcpy(buffer+pos, &index, sizeof(int64_t));
                pos+=sizeof(int64_t);
                pgvector pv = *(struct pgvector *)hashmap_get(hp, j);
                memcpy(buffer+pos, &pv.size, sizeof(pv.size));
                pos+=sizeof(pv.size);
                for(k = 0; k < pv.size; k++){
                    int64_t value = *(int64_t*)vector_get(&pv, k);
                    memcpy(buffer+pos, &value, sizeof(int64_t));
                    pos+=sizeof(int64_t);
                }
            }
        }
    }
}

void hashmap_deserialize(Hashmap* hashp[BPLUS_MAX_LEVEL],  char buffer[]){
    int64_t pos = 0;
    int i = 0, j = 0, k = 0;
    int64_t* tmp = 0;
    size_t* tmp_size;
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        tmp = (int64_t*)(buffer+pos);
        pos+=sizeof(int64_t);
        int64_t hp_size = *tmp;
        if(hp_size > 0){
            for(j = 0; j < hp_size; j++){
                tmp = (int64_t*)(buffer+pos);
                pos+=sizeof(int64_t);
                int64_t key = *tmp;
                pgvector* pv = (pgvector*)malloc(sizeof(pgvector));
                vector_setup(pv, 100, sizeof(int64_t));
                size_t* tmp_size = 0;
                tmp_size = (size_t*)(buffer+pos);
                size_t v_size = *tmp_size;
                pos+=sizeof(pv->size);
                if(v_size > 0){
                    for(k = 0; k < v_size; k++){
                        tmp = (int64_t*)(buffer+pos);
                        pos+=sizeof(int64_t);
                        int64_t value = *tmp;
                        vector_push_back(pv, &value);
                    }
                }
                hashmap_set(hashp[i], key, pv);
            }
        }
    }
    printf("-------")
;}



#endif
