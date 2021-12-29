#include "../def.hpp"

int main(){
    std::ifstream large("./largeUF.txt");
    WeightedQuickUnionwithCompression uf(large);
    return 0;
}