#include <iostream>
#include "Vector.h"
#include "dsa_string.h"
#include "LSD.h"

#include "print.h"

int main(){
    Vector<String> vec;
    vec.insert("4PGC938");
    vec.insert("2IYE230");
    vec.insert("3CI0720");
    vec.insert("1ICK750");
    vec.insert("1OHV845");
    vec.insert("4JXY524");
    vec.insert("1ICK750");
    vec.insert("3CIO720");
    vec.insert("1OHV845");
    vec.insert("1OHV845");
    vec.insert("2RLA629");
    vec.insert("2RLA629");
    vec.insert("3ATW723");
    print(vec);
    LSD(vec, vec.size());
    print(vec);
    std::cout << vec.size() << std::endl;
}
