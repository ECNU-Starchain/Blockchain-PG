#ifndef TESTTREE_H
#define TESTTREE_H
#include"mbtree.h"
#include"vibuilder.h"
#include <unistd.h>

//int count1=0;
//double starttime=0;
//double endtime=0;
//double mytime=0;
//static inline double GetTime()
//{
//    struct timeval tv;
//    gettimeofday(&tv,NULL);
//    return tv.tv_sec + tv.tv_usec / 1e6;
//}

void TestRoot(int64_t root_offset)
{
//test: read the root
    int64_t fd= open("/home/jcq/zcy/opt/treedata/treedata",O_RDWR);
    //cout<<fd<<endl;
    char buf[4096];
    pread(fd,buf,PAGESIZE,root_offset);
    bplus_non_leaf nonleaf;
//    nonleaf.deserialize(buf);
    bplus_non_leaf_deserialize(&nonleaf, buf);
    //cout<<"======================================================begin to print root======================================================"<<endl;
//    nonleaf.print();
    int64_t child1;

//   cout<<"root hash is: "<<nonleaf.node_hash<<endl;
//   cout<<"======================================================begin to print child======================================================"<<endl;
    int i=0;
    for(i=0;i<nonleaf.children;i++)
      {
        child1 =nonleaf.chindrenoffset[i];
        pread(fd,buf,PAGESIZE,child1);
        bplus_leaf* leaf = (bplus_leaf*)malloc(sizeof(bplus_leaf));
        bplus_leaf_init(leaf);
        bplus_leaf_deserialize(leaf,buf);
        //leaf.deserialize(buf);
        printf("ss");
      }
//      cout<<"======================================================print child end======================================================"<<endl;

}

/*
void DiskRangeQuery(bplus_tree*tree, int64_t start, int64_t end, int64_t root_offset)
{

    //test:range query from disk
    vector<int64_t> result;
    bplus_tree_get_range_disk(tree,start,end,&result,root_offset);
//          for(int i =0;i<result.size();i++)
//          {
//              cout<<result.at(i)<<endl;
//          }
    cout<<"result size 1: "<<result.size()<<endl;
}
*/

void AuthRangeQuery(bplus_tree*tree, int64_t start, int64_t end, int64_t root_offset,rangevo *vo)
{
//    vector<int64_t> result;
    pgvector result;
    pgvector key_result;
    int i = 0, j = 0;
    vector_setup(&result, 1000, sizeof(ItemPointer));
    vector_setup(&key_result,1000, sizeof(int64_t));
//    starttime=GetTime();
    bplus_tree_get_range_disk_vo(tree, start,end ,root_offset,&result,vo, &key_result);
//    endtime=GetTime();
//    mytime=mytime + ((double)(endtime-starttime));
//    count1++;
//    if(count1==99){
//       count1=0;
//       mytime=0;
//    }



    ItemPointer* data = (ItemPointer*)malloc(sizeof(ItemPointer) * result.size);
    for(i=0;i<result.size;i++){
        ItemPointer tmp_data = *(ItemPointer*)vector_get(&result, i);
        data[i]=tmp_data;
    }

    int64_t keys[key_result.size];
    for(i=0;i<key_result.size;i++){
        int64_t tmp_key = *(int64_t*)vector_get(&key_result, i);
        keys[i]=tmp_key;
    }

    ItemPointer* tmp = NULL;
/*    int64_t tmp_key;
    for(i=0;i<result.size-1;i++){
        for(j=i+1;j<result.size;j++){
            if(keys[i]>keys[j]){
                tmp=data[i];
                data[i]=data[j];
                data[j]=tmp;

                tmp_key=keys[i];
                keys[i]=keys[j];
                keys[j]=tmp_key;
            }
        }
    }
*/

    int size_count=0;
    for(i=0;i<result.size-1;i++){
        if(data[i]->ip_posid!=data[i+1]->ip_posid){
            data[size_count]=data[i];
            size_count++;
        }
    }
    if(data[result.size-1]->ip_posid<100){
        data[size_count]=data[result.size-1];
        size_count++;
    }
    for(i=0;i<size_count;i++){
        vector_push_back(&vo->itemdata, &data[i]);
    }
    vector_destroy(&result);


//    for(i = 0; i < vo->itemdata.size; i++)
//    {
//        ItemPointer tid= *(ItemPointer*)vector_get(&vo->itemdata, i);
//        printf("-----------------");
//    }
}

int64_t   StoreBtree(bplus_tree*tree)
{
    int64_t root_offset = bplus_tree_store(tree);
    tree->offset = 0;
    return root_offset;
}

#endif // TESTTREE_H
