#include "../def.hpp"  
#include <fstream>
#include <iostream>

using std::cout;

template<typename Tv, typename Te>
void test_tsort(GraphMatrix<Tv, Te>& matrix){
    Stack<Tv>* S = matrix.tSort(0);
    print(S);
}

template<typename Tv, typename Te>
void test_BFS(GraphMatrix<Tv, Te>& matrix){
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
void test_DFS(GraphMatrix<Tv, Te>& matrix){
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

int main(int argc, char** argv){
    
    putchar('\n');

    std::ifstream stream("./files/mediumG.txt");
    GraphMatrix<char,int> tinyG(stream, GType::GRAPH);

    std::cout << "TinyG:  " << std::endl;
    //print(tinyG);
    //bfs
    std::cout << "BFS:  " << std::endl;
    test_BFS(tinyG);
    //print(tinyG);
    std::cout << "==========================" << std::endl;

    //dfs
    std::cout << "DFS:  " << std::endl;
    test_DFS(tinyG);
    //print(tinyG);
    std::cout << "==========================" << std::endl;

    //tSort
    std::cout << "TSort:    " << std::endl;
    test_tsort(tinyG);
    //print(tinyG);

    return 0;
}