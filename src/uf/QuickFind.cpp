#include <fstream>
#include "QuickFind.h"

int main(){
    std::ifstream large("dataset/tinyUF.txt");
    QuickFind uf(large);
    return 0;
}