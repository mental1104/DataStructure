#include "../def.hpp"

int main(int argc, char** argv){
    if(argc != 2){
        printf("Usage %s <num>\n", argv[0]);
        return 0;
    }

    int scale = atoi(argv[1]);

    std::ofstream os("./flawedData.txt");
    Hashtable<int, int> ht;
    Vector<int> vec;

    int offset = 0;

    int oldcapacity;
    //bool flag = true;
    while(ht.size() < scale){

        oldcapacity = ht.capacity();
        ht.put(2*oldcapacity, 2*oldcapacity);
        os << 2*oldcapacity << " ";

        while(oldcapacity == ht.capacity()){
            if(ht.size() == scale)
                break;
            ht.put(oldcapacity+offset, oldcapacity+offset);
            os << oldcapacity+offset << " ";
            offset++;
        }
        offset = 0;
    }
    printf("Done\n");
    //print(vec);
    return 0;
}