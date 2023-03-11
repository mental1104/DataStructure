#include <string>
#include "test_vector.h"

void TestVectorFind();

int main(int argc, char** argv){
    if (argc < 2 || argv[1] == std::string("1"))     
        testVectorFind<int>();
    if (argc < 2 || argv[1] == std::string("2"))     
        testVectorSearch<int>();
}