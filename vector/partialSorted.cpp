#include "Vector.hpp"

//param[1] = method(bubble/selection/insertion), param[2] = length of sequence
int main(int argc, char**argv){
    if(argc!=2){
        printf("Usage: %s <scale>\n", argv[0]);
        return 0;
    }
    Vector<int> vec;
    int scale = atoi(argv[1]);
    int random = scale/10;
    for(int i = 0; i < scale - random; i++)
        vec.insert(i);
    
    for(int i = scale - random; i< scale; i++)
        vec.insert(dice(scale - random), dice(scale));
    
    std::string s("./");
    s = s + argv[1] + "ints.txt";
    std::ofstream output(s);
    for(int i = 0; i < scale; i++)
        output << vec[i] << " ";
    return 0;
}