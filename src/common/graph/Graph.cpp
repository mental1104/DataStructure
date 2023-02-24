#include "../def.hpp"  
#include <fstream>
#include <iostream>

using std::cout;

template<typename Tv, typename Te>
void testTsort(GraphMatrix<Tv, Te>& matrix){
    Stack<Tv>* S = matrix.tSort(0);
    print(S);
    release(S);
}

template<typename Tv, typename Te>
void testBFS(GraphMatrix<Tv, Te>& matrix){
    matrix.bfs(0);
    int n = matrix.n;
    for(int i = 0; i < n; i++){
        List<size_t> path;
        size_t pos = i;
        while(pos != -1){
            path.insertAsFirst(pos);
            pos = matrix.V()[pos].parent;
        }
        print(path);
    }
}

template<typename Tv, typename Te>
void testDFS(GraphMatrix<Tv, Te>& matrix){
    matrix.dfs(0);
    int n = matrix.n;
    for(int i = 0; i < n; i++){
        List<size_t> path;
        size_t pos = i;
        while(pos != -1){
            path.insertAsFirst(pos);
            pos = matrix.V()[pos].parent;
        }
        print(path);
    }
}

template<typename Tv, typename Te, typename PU>
void testPFS(GraphMatrix<Tv, Te>& matrix, PU priorityUpdater, int s){
    matrix.pfs(s, priorityUpdater);
    int n = matrix.n;
    double sum;
    for(int i = 0; i < n; i++){
        sum = 0.0;
        List<size_t> path;
        size_t pos = i;
        while(pos != -1){
            path.insertAsFirst(pos);
            int parent = matrix.V()[pos].parent;
            if(parent != -1)
                sum += matrix.weight(parent, pos);
            pos = parent;
        }
        printf("%d to %d (%4.2f): ", s, i, sum);
        print(path);
    }
}

int main(int argc, char** argv){

    std::ifstream stream("./files/algo/tinyEWG.txt");
    GraphMatrix<int,double> tinyG(stream, GType::WEIGHTEDUNDIGRAPH);
    testPFS(tinyG, PrimPU<int, double>(), 0);
    print(tinyG);
    //tinyG.kruskal(true);
    //testDFS(tinyG);
    //testPFS(tinyG, DijkstraPU<int,double>(), 0);
    
    /*
    std::cout << argv[1] << std::endl;
    //print(tinyG);
    //bfs
    std::cout << "BFS:  " << std::endl;
    test_BFS(tinyG);
    //print(tinyG);
    std::cout << "==========================" << std::endl;

    print(tinyG);
    //dfs
    std::cout << "DFS:  " << std::endl;
    test_DFS(tinyG);
    //print(tinyG);
    std::cout << "==========================" << std::endl;

    print(tinyG);
    //tSort
    std::cout << "TSort:    " << std::endl;
    test_tsort(tinyG);
    //print(tinyG);
    */   
    return 0;
}