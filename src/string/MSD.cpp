#include "Vector.h"
#include "String.h"
#include "MSD.h"
#include <iostream>

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
    // print(vec);
    MSD::sort(vec);
    // print(vec);
    std::cout << vec.size() << std::endl;
    return 0;
}