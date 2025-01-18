#include "../Vector.hpp"

int main(int argc, char** argv){
    if(argc!=2){
        printf("Usage: %s <scale>\n", argv[0]);
        return 0;
    }
    Vector<int> src;
    src.insert(50);
    src.insert(100);
    src.insert(150);
    src.insert(200);

    int scale = atoi(argv[1]);

    int size = src.size();
    Vector<int> vec;
    for(int i = 0; i < scale; i++)
        vec.insert(src[dice(i+1)%size]);
    
    std::string s("./");
    s = s + argv[1] + "Mints.txt";
    std::ofstream output(s);
    for(int i = 0; i < scale; i++)
        output << vec[i] << " ";

    return 0;
}