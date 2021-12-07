#include "./Vector.hpp"
#include <cstdio>

int main(){
    Vector<int> vec;
    std::ifstream stream("./testfiles/10ints.txt");


    int temp;
    while(stream >> temp)
        vec.insert(temp);
    stream.close();

    vec.evaluateSorting();

    return 0;
} 