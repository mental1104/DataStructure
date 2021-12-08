#include "../util/def.hpp"
#include <string>

int main(int argc, char**argv){
    if(argc!=3){
        printf("Usage: %s <num>K\n", argv[0]);
        return 0;
    }

    std::string s("./");
    s = s + argv[1] + argv[2] + "ints.txt";
    int n;
    if(argv[2][0] == 'M')
        n = atoi(argv[1]) * 1000*1000;
    else
        n = atoi(argv[1]) * 1000;

    std::ofstream os(s);
    for(int i = 0; i < n; i++)
        os << dice(-n, n) << '\n';
    return 0;
}