#include "../def.hpp"

int main(){
    std::ifstream large("./largeUF.txt");
    WeightedQuickUnion uf(large);
    return 0;
}