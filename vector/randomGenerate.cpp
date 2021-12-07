#include "../util/def.hpp"
#define N 10
int main(){
    std::ofstream os("./testfiles/10ints.txt");
    for(int i = 0; i < N; i++)
        os << dice(-10, 10) << '\n';
    return 0;
}