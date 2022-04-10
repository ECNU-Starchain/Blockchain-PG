#include "mbt/vibuilder.h"
#include<string.h>
#include <stdlib.h>
#include <stdio.h>
//using namespace std;
char* hashcompute(bplus_node *node){
    if(NULL ==node)
    {
        //cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
        return NULL;
    }
    if (is_leaf(node))
    {
        char* leafstring = (char*)malloc(4096);
        memset(leafstring, 0, 4096);
        bplus_leaf *ln = (struct bplus_leaf *)node;
        int i = 0;
        for(i = 0; i < ln->entries; i++){
            //char* tmp =(char*)ln->data[i];
            char* tmp=(char*)malloc(33);
            memset(tmp, 0, 33);
            //itoa(ln->data[i], tmp, 10);
            //sprintf(tmp, "%d", ln->data[i]);
            strcpy(tmp,ln->data[i]);
            //ln->childhash[i] = sha256(tmp);
            //ln->childhash[i]=(char*)malloc(33);
            sha256_pg(tmp,ln->childhash[i]);
            //leafstring +=ln->childhash[i];
            strcat(leafstring,ln->childhash[i]);
            free(tmp);
        }
        //ln->node_hash=(char*)malloc(33);
        sha256_pg(leafstring,ln->node_hash);
        //ln->print();
        free(leafstring);
        return ln->node_hash;
    } else
    {
        char* nonleafstring= (char*)malloc(4096);
        memset(nonleafstring, 0, 4096);
        char* sub_hash=(char*)malloc(1024);
        memset(sub_hash, 0, 1024);
        struct bplus_non_leaf *nln = (struct bplus_non_leaf *)node;
        int i = 0;
        for(i=0;i<nln->children;i++)
        {
            strcpy(sub_hash ,hashcompute(nln->sub_ptr[i]));
            strcpy(nln->childhash[i], sub_hash);
            //nonleafstring+= sub_hash;
            strcat(nonleafstring,sub_hash);
        }
        //nln->node_hash = sha256((nonleafstring));
        //nln->node_hash=(char*)malloc(33);
        sha256_pg(nonleafstring,nln->node_hash);
        free(nonleafstring);
        free(sub_hash);
        return nln->node_hash;
    }
}

//struct siblinghash
//{
//   int sibling_bumber;
//   string siblinghash[BPLUS_MAX_ENTRIES];
//};

//struct siblingnode
//{
//   siblinghash leftsibling;
//   siblinghash rightsibling;
//};

//struct verification_object{
//        int traverse_height;
//        siblingnode node[BPLUS_MAX_LEVEL];
//        int leafdata;
//};


/*
char* reconstruct_hash(verification_object vo)
{
    int height = vo.traverse_height;
    char* nodehash=(char*)malloc(33);
    sha256_pg("ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssssssssssssssssss"
              "ssssss",nodehash);
//    char* nodehash = sha256("ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssssssssssssssssss"
//                                     "ssssss");
    //cout<<"ffffffffffffffffffffffffffff"<<vo.leafdata<<endl;
    int i = 0;
    for(i = height; i>=0; i--)
    {
        nodehash =calcnodehash(vo.node[i],nodehash);
    }

    //   for(int i = 0;i<=vo.traverse_height;i++)
    //   {
    //     siblingnode node1 =  vo.node[i];

    //     //cout<<"/**********************height: "<<i<<"; sibling hash**************************"<<endl;
    //     siblinghash lh = node1.leftsibling;
    //     for(int j=0;j<lh.sibling_bumber;j++)
    //     {
    //        //cout<<j<<"-th left hash: "<<lh.sibling[j].c_str()<<endl;
    //     }
    //     siblinghash rh = node1.rightsibling;
    //     for(int j=0;j<rh.sibling_bumber;j++)
    //     {
    //        //cout<<j<<"-th right hash: "<<rh.sibling[j].c_str()<<endl;
    //     }
    //   }


    return nodehash;//return root;
}
*/
