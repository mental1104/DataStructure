#include "./Quick3string.hpp"

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
    Quick3string::sort(vec);
    print(vec);
    return 0;
}