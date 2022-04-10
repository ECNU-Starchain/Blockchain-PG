/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "clientmbtree.h"
#include <unistd.h>
#include <pthread.h>
#include <threadpool.h>
#define _BPLUS_TREE_DEBUG;
//using namespace __gnu_cxx;
enum {
    LEFT_SIBLING,
    RIGHT_SIBLING = 1,
};
#define THREAD 32
#define QUEUE  256
pthread_mutex_t lock;
threadpool_t *pool;
static int is_pool=0;
int is_left=0;
int is_right=0;
int count1=0;
double startTime=0;
double endTime=0;
double mytime=0;
//int level_count = 0;
//int node_count=0;
//Hashmap* tmp_hashes[BPLUS_MAX_LEVEL];
static inline double GetTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1e6;
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
            char* result = (char*)vector_get(&myvector1, j);
            size_t len = strlen(result);
            memcpy(buffer+pos, &len, sizeof(len));
            pos+=sizeof(len);
            memcpy(buffer+pos, result, len);
            pos += len;
//            pos+=sizeof(pg_bplus_non_leaf->childhash[i]);
        }
    }
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
        }
    }
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

typedef struct rr {
    int64_t start_id;
    int64_t end_id;
    int level_count;
    int node_count;
    Hashmap* tmp_hashes[BPLUS_MAX_LEVEL];
    rangevo* vo;

}rr;

void* rr_leaf_node(void *arg){
//    pthread_mutex_lock(&lock);
    startTime = GetTime();
    struct rr* para;
    para = (struct rr*) arg;
    int64_t start_id=para->start_id;
    int64_t end_id=para->end_id;
    int level_count=para->level_count;
    int node_count=para->node_count;
    rangevo* vo=para->vo;
    Hashmap* tmp_hashes[BPLUS_MAX_LEVEL];
    int pp = 0;
    for(pp = 0; pp < BPLUS_MAX_LEVEL; pp++){
        tmp_hashes[pp] =para->tmp_hashes[pp] ;
    }
    //calculate hash values node of continuous nodes
    if (start_id==end_id){
        node_count=start_id;
    }else{
        node_count=start_id+1;
    }
    int m = 0;
    for(m=start_id+1;m<end_id; m++)
    {
        char* mhash = (char*)malloc(8192);
        memset(mhash, 0, 8192);
        pgvector myvector2 = *(pgvector*)vector_get(&vo->data, node_count);
        int j = 0;
        for(j =0 ; j < myvector2.size; j++)
        {
            char* tmp_id = *(char**)vector_get(&myvector2, j);
            char* result=(char*)malloc(33);
            sha256_pg(tmp_id,result);
            strcat(mhash,result);
        }
        sha256_pg(mhash,mhash);
        hashmap_set(tmp_hashes[level_count], m, mhash);
        node_count++;
    }
    endTime = GetTime();
    mytime =mytime + ((double)(endTime - startTime)) ;
    count1++;
    if(count1==99){
       printf("\n\n\n\nreconstruct_root_leaf_n times = %f\n\n\n\n",mytime);
       count1=0;
       mytime=0;
     }
 //   pthread_mutex_unlock(&lock);
    is_left=1;
}

char* reconstruct_root(struct Hashmap* hashp[],rangevo vo, char *root)
{
    memset(root,0,33);
    Hashmap* tmp_hashes[BPLUS_MAX_LEVEL];
    int i = 0;
    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        tmp_hashes[i] = hashmap_new(NUM_BUCKETS, myhash, compare);
    }
    //for each level
    int level_count = 0;
    //first construct hash value of leaf nodes
    i=vo.traverse_height;
    int64_t start_id = vo.leftvo[i].node_id;
    int64_t end_id = vo.rightvo[i].node_id;
    //used to calculate which node
    int node_count=0;
    char* hashca = (char*)malloc(33);
    memset(hashca,0,33);
    //calculate hash value of left boundary node
    int k = 0;
    for(k=0;k<vo.leftvo[i].nhash.size;k++)
    {
        char* temp_hash = (char*)vector_get(&vo.leftvo[i].nhash, k);
        strcat(hashca, temp_hash);
    }

    pgvector myvector1 = *(pgvector*)vector_get(&vo.data, node_count);
    int j=0;
    for(j=0;j<myvector1.size;j++)
    {
        char* tmp_id= *(char**)vector_get(&myvector1, j);
        char* result=(char*)malloc(33);
        sha256_pg(tmp_id,result);
        strcat(hashca,result);
    }
    sha256_pg(hashca,hashca);
    hashmap_set(tmp_hashes[level_count], start_id, hashca);

//    startTime = GetTime();
//    //calculate hash values node of continuous nodes
//    if (start_id==end_id){
//        node_count=start_id;
//    }else{
//        node_count=start_id+1;
//    }
//    int m = 0;
//    for(m=start_id+1;m<end_id; m++)
//    {
//        char* mhash = (char*)malloc(8192);
//        memset(mhash, 0, 8192);
//        pgvector myvector2 = *(pgvector*)vector_get(&vo.data, node_count);
//        int j = 0;
//        for(j =0 ; j < myvector2.size; j++)
//        {
//            char* tmp_id = *(char**)vector_get(&myvector2, j);
//            char* result=(char*)malloc(33);
//            sha256_pg(tmp_id,result);
//            strcat(mhash,result);
//        }
//        sha256_pg(mhash,mhash);
//        hashmap_set(tmp_hashes[level_count], m, mhash);
//        node_count++;
//    }
//    endTime = GetTime();
//    mytime =mytime + ((double)(endTime - startTime)) ;
//    count1++;
//    if(count1==99){
//       printf("\n\n\n\nreconstruct_root_leaf_n times = %f\n\n\n\n",mytime);
//       count1=0;
//       mytime=0;
//     }
//==============pthread start====================
//    struct rr para1;
//    para1.start_id=start_id;
//    para1.end_id=(start_id+end_id)/2;
//    para1.level_count=level_count;
//    para1.node_count=node_count;
//    para1.vo=&vo;
//    int ii = 0;
//    for(ii = 0; ii < BPLUS_MAX_LEVEL; ii++){
//        para1.tmp_hashes[ii] =tmp_hashes[ii] ;
//    }
//    struct rr para2;
//    para2.start_id=(start_id+end_id)/2+1;
//    para2.end_id=end_id;
//    para2.level_count=level_count;
//    para2.node_count=node_count;
//    para2.vo=&vo;
//    int iii = 0;
//    for(iii = 0; iii < BPLUS_MAX_LEVEL; iii++){
//        para2.tmp_hashes[iii] =tmp_hashes[iii] ;
//    }
//    pthread_t thread1;
//    int ret_thrd1;
//    pthread_t thread2;
//    int ret_thrd2;
//    ret_thrd1 = pthread_create(&thread1, NULL, (void *) &rr_leaf_node,(void *) &para1);
//    ret_thrd2 = pthread_create(&thread2, NULL, (void *) &rr_leaf_node,(void *) &para2);
//    pthread_join(thread1, NULL);
//    pthread_join(thread2, NULL);
//==============pthread end====================

//==============static pthread start============
    struct rr para1;
    para1.start_id=start_id;
    para1.end_id=(start_id+end_id)/2;
    para1.level_count=level_count;
    para1.node_count=node_count;
    para1.vo=&vo;
    int ii = 0;
    for(ii = 0; ii < BPLUS_MAX_LEVEL; ii++){
        para1.tmp_hashes[ii] =tmp_hashes[ii] ;
    }
    struct rr para2;
    para2.start_id=(start_id+end_id)/2;
    para2.end_id=end_id;
    para2.level_count=level_count;
    para2.node_count=node_count;
    para2.vo=&vo;
    int iii = 0;
    for(iii = 0; iii < BPLUS_MAX_LEVEL; iii++){
        para2.tmp_hashes[iii] =tmp_hashes[iii] ;
    }
    pthread_t thread1;
    int ret_thrd1;
    pthread_t thread2;
    int ret_thrd2;
    ret_thrd1 = pthread_create(&thread1, NULL, (void *) &rr_leaf_node,(void *) &para1);
    ret_thrd2 = pthread_create(&thread2, NULL, (void *) &rr_leaf_node,(void *) &para2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
//==============static pthraed end==============


//==============pthreadpool end====================
//        struct rr para1;
//        para1.start_id=start_id;
//        para1.end_id=(start_id+end_id)/2;
//        para1.level_count=level_count;
//        para1.node_count=node_count;
//        para1.vo=&vo;
//        int ii = 0;
//        for(ii = 0; ii < BPLUS_MAX_LEVEL; ii++){
//            para1.tmp_hashes[ii] =tmp_hashes[ii] ;
//        }
//        struct rr para2;
//        para2.start_id=(start_id+end_id)/2;
//        para2.end_id=end_id;
//        para2.level_count=level_count;
//        para2.node_count=node_count;
//        para2.vo=&vo;
//        int iii = 0;
//        for(iii = 0; iii < BPLUS_MAX_LEVEL; iii++){
//            para2.tmp_hashes[iii] =tmp_hashes[iii] ;
//        }
//        threadpool_t *pool;
//        if(is_pool==0){
//           pool = threadpool_create(THREAD, QUEUE, 0);
//           is_pool=1;
//        }

//        threadpool_add(pool, (void *)&rr_leaf_node,  (void *) &para1, 0) ;
//        threadpool_add(pool, (void *)&rr_leaf_node,  (void *) &para2, 0) ;
//        while(is_left==0){
//        }
//        is_left=0;

//==============pthreadpool end====================

    //calculate hash value of right boundary node
    char* ehash = (char*)malloc(8192);
    memset(ehash, 0, 8192);
    if(start_id!=0&&end_id!=0&& start_id==end_id &&  vo.rightvo[i].nhash.size!=0 ){
        pgvector myvector2 = *(pgvector*)vector_get(&vo.data, node_count-1);
        for(j =0 ; j<myvector2.size;j++)
        {
            char* tmp_id = *(char**)vector_get(&myvector2, j);
            char* result=(char*)malloc(33);
            sha256_pg(tmp_id,result);
            strcat(ehash,result);
        }
    }else{
        pgvector myvector2 = *(pgvector*)vector_get(&vo.data, node_count);
        for(j =0 ; j<myvector2.size;j++)
        {
            char* tmp_id = *(char**)vector_get(&myvector2, j);
            char* result=(char*)malloc(33);
            sha256_pg(tmp_id,result);
            strcat(ehash,result);
        }
    }

    for(j=0;j< vo.rightvo[i].nhash.size;j++)
    {
        char *my_hash = (char*)vector_get(&vo.rightvo[i].nhash, j);
        strcat(ehash, my_hash);
    }
    sha256_pg(ehash,ehash);
    hashmap_set(tmp_hashes[level_count], end_id, ehash);
//    startTime = GetTime();
    //for non leaf node
    for(k=1;k<=vo.traverse_height;k++)
    {
        i--;
        start_id = vo.leftvo[i].node_id;
        int64_t end_id = vo.rightvo[i].node_id;
        char* hashca=(char*)malloc(8192);
        memset(hashca,0,8192);

        if(start_id !=end_id)
        {
            //calculate hash value of left boundary node
            int j = 0;
            for(j=0;j<vo.leftvo[i].nhash.size;j++)
            {
                char *my_hash = (char*)vector_get(&vo.leftvo[i].nhash, j);
                strcat(hashca, my_hash);
            }
            //get each children of the node
            pgvector pv = *(struct pgvector *)hashmap_get(hashp[k], start_id);

            int vsize = pv.size;
            for(j=0; j< vsize;j++)
            {
                int64_t child_id = *(int64_t*)vector_get(&pv, j);
                char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], child_id);
                if(ss_temp != NULL){
                    strcat(hashca, ss_temp /*tmp_hashes[level_count][child_id]*/);
                }
            }

            //calculate hash value of this node
            char *tmp_hashca = (char*)malloc(33);
            memset(tmp_hashca, 0, 33);
            sha256_pg(hashca,tmp_hashca);
            free(hashca);
            hashmap_set(tmp_hashes[level_count+1], start_id, tmp_hashca);

            //calculate hash values node of continuous nodes
            int p = 0;
            for(p=start_id+1;p<end_id;p++)
            {
                char* phash=(char*)malloc(32);
                memset(phash,0,32);
                pgvector childs = *(struct pgvector *)hashmap_get(hashp[k], p);
                int i = 0;
                for(i =0;i<childs.size;i++)
                {
                    int64_t index = *(int64_t*)vector_get(&childs, i);
                    char* ss_temp = (char*)hashmap_get(tmp_hashes[level_count], index);
                    strcat(phash,  ss_temp);/*tmp_hashes[level_count][index]*/
                }
                char *tmp_phash = (char*)malloc(33);
                memset(tmp_phash, 0, 33);
                sha256_pg(phash,tmp_phash);
                free(phash);
                hashmap_set(tmp_hashes[level_count+1], p, tmp_phash);
            }


            //calculate hash value of right boundary node
            char* ehash2=(char*)malloc(8192);
            memset(ehash2, 0, 8192);
            pgvector pv1 = *(struct pgvector *)hashmap_get(hashp[k], end_id);
            for(j =0 ; j < pv1.size;j++)
            {
                int64_t child_id = *(int64_t*)vector_get(&pv1, j);
                char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], child_id);
                if(ss_temp != NULL){
                    strcat(ehash2,ss_temp);
                }
            }

            for(j=0;j<vo.rightvo[i].nhash.size;j++)
            {
                 char *my_hash = (char*)vector_get(&vo.rightvo[i].nhash, j);
                 strcat(ehash2, my_hash);
            }
            sha256_pg(ehash2,ehash2);
            hashmap_set(tmp_hashes[level_count+1], end_id, ehash2);
            level_count++;
        }
        else
        {
            int j = 0;
            //calculate hash value of left boundary node
            for(j=0;j<vo.leftvo[i].nhash.size;j++)
            {
                 char *my_hash = (char*)vector_get(&vo.leftvo[i].nhash, j);
                strcat(hashca,my_hash);
            }
            //get each children of the node
            pgvector pv2 = *(struct pgvector *)hashmap_get(hashp[k], start_id);
            for(j=0;j<pv2.size;j++)
            {
                int64_t child_id = *(int64_t*)vector_get(&pv2, j);
                char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], child_id);
                if(ss_temp != NULL){
                     strcat(hashca,ss_temp);
                }
            }

            for(j=0;j<vo.rightvo[i].nhash.size;j++)
            {
                char *my_hash = (char*)vector_get(&vo.rightvo[i].nhash, j);
                strcat(hashca, my_hash);
            }
            char *tmp_hashca = (char*)malloc(33);
            memset(tmp_hashca, 0, 33);
            sha256_pg(hashca, tmp_hashca);
            free(hashca);
            hashmap_set(tmp_hashes[level_count+1], end_id, tmp_hashca);
            level_count++;
        }
    }
    char *ss_temp = (char*)hashmap_get(tmp_hashes[level_count], 0);
    strcpy(root , ss_temp);
//    endTime = GetTime();
//    mytime =mytime + ((double)(endTime - startTime)) ;
//    count1++;
//    if(count1==99){
//       printf("\n\n\n\nreconstruct_root_nonleaf times = %f\n\n\n\n",mytime);
//       count1=0;
//       mytime=0;
//    }

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
            for(j = 0; j < 100; j++){
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
}


