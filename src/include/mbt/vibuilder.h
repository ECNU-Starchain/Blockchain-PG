#ifndef MBTREEBUILDER_H
#define MBTREEBUILDER_H
//#include<iostream>
#include "sha256.h"
#include "mbtree.h"
//using namespace std;
char* hashcompute(bplus_node *node);
//char*  calcnodehash(siblingnode &node,char*  data);
char*  reconstruct_hash(verification_object vo);
#endif // MBTREEBUILDER_H
