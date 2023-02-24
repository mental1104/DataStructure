#include "../def.hpp"
 /*
  * para[1] = scale 
  * para[2] = method : 0-AVL/1-RedBlack/2-Splay
  * 
  */
int main(int argc, char** argv){
    if(argc!=2){
        printf("Usage: %s <num> <method>", argv[0]);
        return 0;
    }

    int method = atoi(argv[1]);

    std::ifstream flawedData("./flawedData.txt");
    
    sleep(1);

    clock_t start, end;
    Hashtable<int, int> ht;
    QuadraticHT<int, int> qd;
    int temp;

    start = clock();
    switch(method){
        case 0: 
            while(flawedData >> temp)
                ht.put(temp, temp);
            break;
        case 1:
            while(flawedData >> temp)
                qd.put(temp, temp);
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