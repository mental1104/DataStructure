#include <iostream>
#include <fstream>

using namespace::std;

int main(){
    ifstream is("./mediumEWG.txt");
    ofstream os("./medium.txt");

    int v;
    int e;
    is >> v >> e;
    int origin, dest;
    double weight;
    os << v << " " << e << "\n";
    while(is >> origin >> dest >> weight){
        os << origin << " " << dest << " " << weight << " ";
    }
    return 0;
}