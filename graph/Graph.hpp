#pragma once

#include "../stack/Stack.hpp"
#include "../queue/Queue.hpp"

#include <climits>
enum class VStatus {
    SOURCE,
    UNDISCOVERED,
    DISCOVERED,
    VISITED
};//顶点状态

enum class EType {
    UNDETERMINED,
    TREE,
    CROSS,
    FORWARD,
    BACKWARD
};//边状态

enum class GType {
    UNDIGRAPH,
    DIGRAPH,
    WEIGHTEDGRAPH
};

template<typename Tv>
struct Vertex {
    Tv data;
    int inDegree{0};
    int outDegree{0};
    VStatus status{VStatus::UNDISCOVERED};
    int dTime{-1};
    int fTime{-1};
    int parent{-1};
    int priority{INT_MAX};
    Vertex(Tv const& d = Tv(0)):data{d}{}
};

template<typename Te>
struct Edge {
    Te data;
    double weight;
    EType type{EType::UNDETERMINED};
    Edge(Te const& d, double w):data{d}, weight{w} {}
};

template<typename Tv, typename Te>
class Graph {
private:
    void reset(){
        for(int i = 0; i < n; i++){
            status(i) = VStatus::UNDISCOVERED;
            dTime(i) = -1;
            fTime(i) = -1;
            parent(i) = -1;
            priority(i) = INT_MAX;
            for(int j = 0; j < n; j++)
                if(exists(i,j))
                    type(i, j) = EType::UNDETERMINED;
        }
    }
    void BFS(int, int& );//广度优先搜索
    void DFS(int, int&);//深度优先搜索
    void BCC(int, int&, Stack<int>& );//双连通分量分解
    bool TSort(int, int&, Stack<Tv>* );//拓扑排序
    template<typename PU> void PFS(int, PU);//优先级搜索
    void CC(int, Vector<int>&, Vector<bool>&, int&);//连通分量
    
public:
    //顶点接口
    int n;
    virtual int insert(Tv const& ) = 0;//插入顶点，返回编号
    virtual Tv remove(int) = 0;//删除顶点及其关联边，返回顶点信息
    virtual Tv& vertex(int) = 0;//顶点v的数据
    virtual int inDegree(int) = 0;//顶点v的入度
    virtual int outDegree(int) = 0;//顶点v的出度
    virtual int firstNbr(int) = 0;//顶点v的首个邻接顶点
    virtual int nextNbr(int, int) = 0;//顶点v的下一个邻接顶点
    virtual VStatus& status(int) = 0;
    virtual int& dTime(int) = 0;//时间标签
    virtual int& fTime(int) = 0;
    virtual int& parent(int) = 0;
    virtual int& priority(int) = 0;
    //边接口
    int e;
    virtual bool exists(int, int) = 0;
    virtual void insert(Te const& edge, int, int, int) = 0;//在顶点v和u之间插入权重为w的边e
    virtual Te remove(int, int) = 0;
    virtual EType& type(int, int) = 0;
    virtual Te& edge(int, int) = 0;
    virtual double& weight(int, int) = 0;
    //算法
    void bfs(int);//广度优先搜索
    void dfs(int);//深度优先搜索
    void vcc(int);//双连通分量分解
    Stack<Tv>* tSort(int);//基于DFS的拓扑排序
    void prim(int);//最小生成树
    void dijkstra(int);//最短路径
    template<typename PU> void pfs(int, PU);
    int connectedComponents(bool flag = false);
    bool connectedComponents(int v, int w);
}; 

template<typename Tv, typename Te>
void Graph<Tv, Te>::bfs(int s) {
    reset();
    int clock = 0;
    int v = s;
    do 
        if(VStatus::UNDISCOVERED == status(v)){//如果没有访问过
            BFS(v, clock); //执行BFS
            status(v) = VStatus::SOURCE;//第一个节点设为起点
        }
    while(s != (v = (++v%n)));
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::BFS(int v, int& clock) {
    Queue<int> Q;
    status(v) = VStatus::DISCOVERED;
    Q.enqueue(v);
    while(!Q.empty()) {
        int v = Q.dequeue();
        dTime(v) = ++clock;
        for(int u = firstNbr(v); u < this->n; u = nextNbr(v, u))
            if(VStatus::UNDISCOVERED == status(u)){
                status(u) = VStatus::DISCOVERED;
                Q.enqueue(u);
                type(v, u) = EType::TREE;
                parent(u) = v;
            } else {
                type(v, u) = EType::CROSS;
            }

        if(status(v)!=VStatus::SOURCE)
            status(v) = VStatus::VISITED;
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::dfs(int s){
    reset();
    int clock = 0;
    int v = s;
    do
    {
        if(VStatus::UNDISCOVERED == status(v)){
            DFS(v, clock);
            status(v) = VStatus::SOURCE;
        }
            
    } while (s != (v = (++v % n)));  
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::DFS(int v, int& clock) {
    dTime(v) = ++clock;
    status(v) = VStatus::DISCOVERED;
    for(int u = firstNbr(v); u < this->n; u = nextNbr(v, u))
        switch(status(u)){
            case VStatus::UNDISCOVERED:
                type(v, u) = EType::TREE;
                parent(u) = v;
                DFS(u, clock);
                break;
            case VStatus::DISCOVERED:
                type(v, u) = EType::BACKWARD;
                break;
            default:
                type(v, u) = (dTime(v) < dTime(u))?EType::FORWARD:EType::CROSS;
                break;
        }
    status(v) = VStatus::VISITED;
    fTime(v) = ++clock;
}

template<typename Tv, typename Te>
Stack<Tv>* Graph<Tv, Te>::tSort(int s){
    reset();
    int clock = 0;
    int v = s;
    Stack<Tv> S = new Stack<Tv>;
    do {
        if(VStatus::UNDISCOVERED == status(v))
            if(!TSort(v, clock, S)){
                while(!S->empty())
                    S->pop();
                break;
            }
    } while(s != (v = (++v % n)));
    return S;
}

template<typename Tv, typename Te>
bool Graph<Tv, Te>::TSort(int v, int& clock, Stack<Tv>* S){
    dTime(v) = ++clock;
    status(v) = VStatus::DISCOVERED;
    for(int u = firstNbr(v); u < this->n; u = nextNbr(v, u))
        switch(status(u)){
            case VStatus::UNDISCOVERED:
                parent(u) = v;
                type(v, u) = EType::TREE;
                if(!TSort(u, clock, S))
                    return false;
                break;
            case VStatus::DISCOVERED:
                type(v, u) = EType::BACKWARD;
                return false;
            default:
                type(v, u) = (dTime(v) < dTime(u)) ? EType::FORWARD : EType::CROSS;
                break;
        }
    status(v) = VStatus::VISITED;
    S->push(vertex(v));
    return true;
}

template<typename Tv, typename Te>
int Graph<Tv, Te>::connectedComponents(bool flag){
    Vector<bool> marked{this->n,this->n, false};
    Vector<int> id{this->n, this->n, 0};
    int count = 0;

    for(int s = 0; s < this->n; s++){
        if(!marked[s])
        {
            CC(s, id, marked, count);
            count++;
        }
    }

    if(flag){
        for(int i = 0; i < count; i++){
            for(int j = 0; j < id.size(); j++){
                if(id[j] == i)
                    printf("%d ", j);
            }
            printf("\n");
        }
    }
    return count;
}

template<typename Tv, typename Te>
bool Graph<Tv, Te>::connectedComponents(int v, int w){
    Vector<bool> marked{this->n,this->n, false};
    Vector<int> id{this->n, this->n, 0};
    int count = 0;

    for(int s = 0; s < this->n; s++){
        if(!marked[s])
        {
            CC(s, id, marked, count);
            count++;
        }
    }
    return id[v] == id[w];
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::CC(int v, Vector<int>& id, Vector<bool>& marked, int& count){
    marked[v] = true;
    id[v] = count;
    for(int u = firstNbr(v); u < this->n; u = nextNbr(v, u)){
        if(!marked[u]){
            CC(u, id, marked, count);
        }
    }
}