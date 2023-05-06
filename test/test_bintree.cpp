#include "test_helper.h"

template <typename T> 
void testBinTreeRangefor () {
    BinTree<T> bintree = generateBinTree<T>();

    std::ostringstream ss1;
    std::ostringstream ss2;

    for(auto& i : bintree)
        ss1 << i.data << " ";
    traverse(bintree.root(), ss2);

    std::string res1 = ss1.str();
    std::string res2 = ss2.str();

    std::cout << res1 << std::endl;
    std::cout << res2 << std::endl;

    if(res1 != res2)
        exit(1);

    return;
}

template <typename T>
void testBinTreeCoroutineGenerator(){

}

int main(){
    testBinTreeRangefor<int>();
}