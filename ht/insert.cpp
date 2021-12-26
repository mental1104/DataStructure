#include "../def.hpp"
 /*
  * para[1] = scale 
  * para[2] = method : 0-AVL/1-RedBlack/2-Splay
  * 
  */
int main(int argc, char** argv){
    if(argc!=3){
        printf("Usage: %s <num> <method>", argv[0]);
        return 0;
    }

    int scale = atoi(argv[1]);
    int method = atoi(argv[2]);

    Vector<int> vec;
    for(int i = 0; i < scale; i++)
        vec.insert(i);
    vec.unsort();
    sleep(1);

    clock_t start, end;
    Hashtable<int, int> ht;
    QuadraticHT<int, int> qd;

    start = clock();
    switch(method){
        case 0: 
            for(int i = 0; i < scale; i++){
                int val = vec[i];
                ht.put(val, val);
            }
            break;
        case 1:
            for(int i = 0; i < scale; i++){
                int val = vec[i];
                qd.put(val, val);
            }
            break;
        
        default:
            exit(-1);
    }
    end = clock();

    std::ofstream output;
    output.open("./data.txt", std::ios::app | std::ios::out);
    output << double(end-start)/CLOCKS_PER_SEC << " ";
    
    if(method == 0)
        printf("Insert: Linear Probing size: %d\n", ht.size());
    else if(method == 1)
        printf("Insert: Quadratic Probing size: %d\n", qd.size());
        
    return 0;
}