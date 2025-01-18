#include <iostream>

#include "Vector.h"
#include "String.h"
#include "Quick3String.h"
#include "print.h"

int main(){
    Vector<String> vec;
    vec.insert("she");
    vec.insert("sells");
    vec.insert("seashells");
    vec.insert("by");
    vec.insert("the");
    vec.insert("sea");
    vec.insert("shore");
    vec.insert("the");
    vec.insert("shells");
    vec.insert("she");
    vec.insert("sells");
    vec.insert("are");
    vec.insert("surely");
    vec.insert("seashells");
    print(vec);
    Quick3String::sort(vec);
    print(vec);
    std::cout << vec.size() << std::endl;
    return 0;
}