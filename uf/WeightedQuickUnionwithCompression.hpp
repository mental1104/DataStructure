#pragma once 
#include "../def.hpp"

class WeightedQuickUnionwithCompression : public WeightedQuickUnion {
public:
    WeightedQuickUnionwithCompression() = delete;
    WeightedQuickUnionwithCompression(int N) : WeightedQuickUnion(N){}
    WeightedQuickUnionwithCompression(std::ifstream& is) : WeightedQuickUnion(is){}
    virtual ~WeightedQuickUnionwithCompression(){}

    virtual int find(int p);
};

//Amortized O(1). Less than 5 for any conceivable practical value of N
int WeightedQuickUnionwithCompression::find(int p){
    int root = p;
    while(root != _id[root]){
        root = _id[root];
    }

    while(p != _id[p]){
        int NextParent = _id[p];
        _id[p] = root;
        p = NextParent;
    }

    return root;
}