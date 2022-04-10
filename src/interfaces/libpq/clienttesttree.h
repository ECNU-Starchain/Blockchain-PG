#ifndef TESTTREE_H
#define TESTTREE_H
#include"clientmbtree.h"
#include"clientvibuilder.h"
#include <unistd.h>


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
    //vector_setup(&result, 10, sizeof(int64_t));
    vector_setup(&result, 60, sizeof(ItemPointer));
    bplus_tree_get_range_disk_vo(tree, start,end ,root_offset,&result,vo);
    printf("%s%d\n","result size is: ",result.size);
}

int64_t   StoreBtree(bplus_tree*tree)
{
    int64_t root_offset = bplus_tree_store(tree);
    tree->offset= 0;
    return root_offset;
}

#endif // TESTTREE_H
