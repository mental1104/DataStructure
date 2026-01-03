#ifndef __DSA_GRAPH
#define __DSA_GRAPH

#include "utils.h"
#include "Vector.h"
#include "Stack.h"
#include "Queue.h"
#include "Heap.h"
#include "WeightedQuickUnionwithCompression.h"
#include "GraphObserver.h"
#include <climits>
#include <limits>

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
    WEIGHTEDUNDIGRAPH,
    WEIGHTEDDIGRAPH
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
    double priority{std::numeric_limits<double>::infinity()};
    int rank{-1};
    Vertex() = default;
    Vertex(int r, Tv const& d = Tv(0)):data(d), rank(r) {}
    bool operator<(const Vertex<Tv>& rhs) const { return priority > rhs.priority;  }//权重越小，优先级越高
};

template<typename Te>
struct Edge {
    Te data;
    double weight;
    EType type{EType::UNDETERMINED};
    int x;
    int y;
    Edge() = default;
    Edge(Te const& d, double w, int i, int j):data{d}, weight{w}, x(i), y(j){}
    bool operator<(const Edge<Te>& rhs) const { return weight > rhs.weight;  }//权重越小，优先级越高
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

            priority(i) = std::numeric_limits<double>::infinity(); // Windows 的 unsigned long 是 32 位，之前的位模式常量会截断，改用标准写法跨平台

            for(int j = 0; j < n; j++)
                if(exists(i,j))
                    type(i, j) = EType::UNDETERMINED;
        }
    }
    void BFS(int, int& );//广度优先搜索
    void DFS(int, int&);//深度优先搜索
    void BCC(int, int&, Stack<int>& );//双连通分量分解
    void TSort(int, int&, Stack<Tv>* );//拓扑排序
    template<typename PU> void PFS(int, PU);//优先级搜索

    void Cycle(int, int, bool&, Vector<bool>&);//无向图有环遍历
    bool cycle();//无向图判断是否有环

    void DirectedCycle(int, Vector<bool>&, Vector<int>&, Stack<int>&, Vector<bool>&);//有向图有环遍历
    bool directedCycle(bool flag = false);//有向图判断是否有环

    void ReversePost(int, Vector<bool>&, Stack<int>*&);
    Stack<int>* reversePost();//给出逆序的拓扑排序序列（没有必须无环的限制）

    void CC(int, Vector<int>&, Vector<bool>&, int&);//无向图-连通分量遍历
    void RC(int, Vector<bool>&);//有向图-可达分量遍历

    void KosarajuSCC(int, int&, Vector<bool>&, Vector<int>&);//强连通子图遍历

public:
    GraphObserver<Tv, Te>* observer{nullptr};
    void setObserver(GraphObserver<Tv, Te>* obs) { observer = obs; }

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
    virtual int& fTime(int) = 0;//bcc时间标签
    virtual int& hca(int) = 0;
    virtual int& parent(int) = 0;
    virtual double& priority(int) = 0;
    //边接口
    int e;
    virtual bool exists(int, int) = 0;
    virtual void insert(Te const& edge, int, int, double) = 0;//在顶点v和u之间插入权重为w的边e
    virtual Te remove(int, int) = 0;
    virtual EType& type(int, int) = 0;
    virtual Te& edge(int, int) = 0;
    virtual double& weight(int, int) = 0;
    virtual void reverse() = 0;
    //算法
    void bfs(int);//广度优先搜索
    void dfs(int);//深度优先搜索
    void bcc(int);//双连通分量分解
    Stack<Tv>* tSort(int);//基于DFS的拓扑排序
    void prim(int);//最小生成树
    void dijkstra(int);//最短路径
    template<typename PU> void pfs(int, PU);
    void kruskal(bool flag = false);

    int connectedComponents(bool flag = false);//无向图-连通分量生成
    bool connectedComponents(int v, int w);//无向图-判断两点是否连通

    void reachableComponents(int s);//有向图-可达分量生成

    int kosarajuSCC(bool flag = false);//有向图-强连通子图统计

}; 

template<typename Tv, typename Te>
void Graph<Tv, Te>::bfs(int s) {
    reset();
    if (n <= 0) return;
    int clock = 0;
    int v = s;
    do {
        if(VStatus::UNDISCOVERED == status(v)){//如果没有访问过
            BFS(v, clock); //执行BFS
            status(v) = VStatus::SOURCE;//第一个节点设为起点
        }
        v = (v + 1) % n;
    }
    while(v != s);
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::BFS(int v, int& clock) {
    Queue<int> Q;
    status(v) = VStatus::DISCOVERED;
    Q.enqueue(v);
    while(!Q.empty()) {
        int cur = Q.dequeue();
        dTime(cur) = ++clock;
        for(int u = firstNbr(cur); -1 < u; u = nextNbr(cur, u)){
            if (!exists(cur, u)) continue;
            if(VStatus::UNDISCOVERED == status(u)){
                status(u) = VStatus::DISCOVERED;
                Q.enqueue(u);
                type(cur, u) = EType::TREE;
                parent(u) = cur;
            } else {
                type(cur, u) = EType::CROSS;
            }
        }

        if(status(cur)!=VStatus::SOURCE)
            status(cur) = VStatus::VISITED;
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::dfs(int s){
    reset();
    if (n <= 0) return;
    int clock = 0;
    int v = s;
    do {
        if(VStatus::UNDISCOVERED == status(v)){
            DFS(v, clock);
            status(v) = VStatus::SOURCE;
        }
        v = (v + 1) % n;
    } while (v != s);  
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::DFS(int v, int& clock) {
    dTime(v) = ++clock;
    status(v) = VStatus::DISCOVERED;
    for(int u = firstNbr(v); -1 < u; u = nextNbr(v, u)){
        if (!exists(v, u)) continue;
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
    }
    status(v) = VStatus::VISITED;
    fTime(v) = ++clock;
}

template<typename Tv, typename Te>
Stack<Tv>* Graph<Tv, Te>::tSort(int s){
    reset();
    if (n <= 0) return new Stack<Tv>;
    
    int clock = 0;
    int v = s;
    Stack<Tv>* S = new Stack<Tv>;

    
    if(directedCycle()){
        return S;
    }

    do {
        if(VStatus::UNDISCOVERED == status(v))
            TSort(v, clock, S);  
        v = (v + 1) % n;
    } while(v != s);

    return S;
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::TSort(int v, int& clock, Stack<Tv>* S){
    dTime(v) = ++clock;
    status(v) = VStatus::DISCOVERED;
    for(int u = firstNbr(v); -1 < u; u = nextNbr(v, u)){
        if (!exists(v, u)) continue;
        switch(status(u)){
            case VStatus::UNDISCOVERED:
                parent(u) = v;
                type(v, u) = EType::TREE;
                TSort(u, clock, S);
                break;
            case VStatus::DISCOVERED:
                type(v, u) = EType::BACKWARD;
                return;
            default:
                type(v, u) = (dTime(v) < dTime(u)) ? EType::FORWARD : EType::CROSS;
                break;
        }
    }
    status(v) = VStatus::VISITED;
    S->push(vertex(v));
    return;
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

    if(flag && observer){
        for(int i = 0; i < count; i++){
            Vector<int> comp;
            for(int j = 0; j < id.size(); j++){
                if(id[j] == i) comp.insert(j);
            }
            observer->onSCCComponent(comp);
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
    for(int u = firstNbr(v); -1 < u; u = nextNbr(v, u)){
        if(!marked[u]){
            CC(u, id, marked, count);
        }
    }
}

//Digraph - reachability in digraphs in Algorithm 4.4
template<typename Tv, typename Te>
void Graph<Tv, Te>::reachableComponents(int s){
    Vector<bool> marked(this->n, this->n, false);
    RC(s, marked);


    if(observer){
        Vector<int> reach;
        for(int i = 0; i < this->n; i++){
            if(marked[i]) reach.insert(i);
        }
        observer->onSCCComponent(reach);
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::RC(int v, Vector<bool>& marked){
    marked[v] = true;
    for(int w = firstNbr(v); -1 < w; w = nextNbr(v, w)){
        if(!marked[w])
            RC(w, marked);

    }
    return;
}

//undigraph cycle
template<typename Tv, typename Te>
bool Graph<Tv, Te>::cycle(){
    bool c = false;
    Vector<bool> marked{this->n, this->n, false};
    for(int s = 0; s < this->n; s++)
        if(!marked[s])
            Cycle(s, s, c, marked);

    return c;
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::Cycle(int v, int u, bool& c, Vector<bool>& marked){
    marked[v] = true;
    for(int w = firstNbr(v); -1 < w; w = nextNbr(v, w))
        if(!marked[w])
            Cycle(w, v, c, marked);
        else if(w != u)
            c = true;
}

template<typename Tv, typename Te>
bool Graph<Tv, Te>::directedCycle(bool flag){
    Vector<bool> marked{this->n, this->n, false};
    Vector<int> edgeTo{this->n, this->n, -1};
    Stack<int> cycle;
    Vector<bool> onStack(this->n, this->n, false);

    for(int v = 0; v < this->n; v++){
        if(!marked[v]) DirectedCycle(v, marked, edgeTo, cycle, onStack);
    }

    if(flag){
        // print(cycle);
    }

    bool res = false;
    if(!cycle.empty()){
        res = true;
    }

    return res;
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::DirectedCycle(
    int v, 
    Vector<bool>& marked, 
    Vector<int>& edgeTo, 
    Stack<int>& cycle, 
    Vector<bool>& onStack)
{
    onStack[v] = true;
    marked[v] = true;

    
    for(int w = firstNbr(v); -1 < w; w = nextNbr(v, w)){
        if(!cycle.empty()){
            return;
        }
        else if(marked[w] == false){
            edgeTo[w] = v;
            DirectedCycle(w, marked, edgeTo, cycle, onStack);
        } else if(onStack[w]){
            for(int x = v; x != w; x = edgeTo[x])
                cycle.push(x);
            cycle.push(w);
            cycle.push(v);
        }
    }
    onStack[v] = false;
}

//output a reverse sequence
template<typename Tv, typename Te>
Stack<int>* Graph<Tv, Te>::reversePost(){
    Stack<int>* order = new Stack<int>();
    Vector<bool> marked(this->n, this->n, false);
    for(int v = 0; v < this->n; v++)
        if(!marked[v])
            ReversePost(v, marked, order);
    return order;
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::ReversePost(int v, Vector<bool>& marked, Stack<int>*& order){
    marked[v] = true;
    for(int w = firstNbr(v); -1 < w; w = nextNbr(v, w)){
        if(!marked[w])
            ReversePost(w, marked, order);
    }
    order->push(v);
}

template<typename Tv, typename Te>
int Graph<Tv, Te>::kosarajuSCC(bool flag){
    if(!directedCycle()){
        return 0;
    }
    
    Vector<bool> marked(this->n, this->n, false);
    Vector<int> id(this->n, this->n, -1);
    int count = 0;
    reverse();
    Stack<int>* order = reversePost();
    reverse();
    int size = order->size();
    for(int i = 0; i < size; i++){
        int s = order->pop();
        if(!marked[s]){
            KosarajuSCC(s, count, marked, id);
            ++count;
        }
    }
    
    if(flag && observer){
        Vector<Vector<int>> comps(count, count, Vector<int>());
        for (int v = 0; v < this->n; v++) {
            if (id[v] >= 0 && id[v] < count) comps[id[v]].insert(v);
        }
        for (int i = 0; i < comps.size(); i++) observer->onSCCComponent(comps[i]);
    }
    return count;
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::KosarajuSCC(int v, int& count, Vector<bool>& marked, Vector<int>& id){
    marked[v] = true;
    id[v] = count;
    for(int w = firstNbr(v); -1 < w; w = nextNbr(v, w)){
        if(!marked[w])
            KosarajuSCC(w, count, marked, id);
    }
}

template <typename Tv, typename Te> 
void Graph<Tv, Te>::bcc(int s){
    reset(); int clock = 0; int v = s; Stack<int> S; //栈S用以记录已访问的顶点
    if (n <= 0) return;
    do {
        if (VStatus::UNDISCOVERED == status(v)) { //一旦发现未发现的顶点（新连通分量）
            BCC ( v, clock, S ); //即从该顶点出发启动一次BCC
            S.pop(); //遍历返回后，弹出栈中最后一个顶点——当前连通域的起点
        }
        v = (v + 1) % n;
    } while(v != s);
}



template <typename Tv, typename Te> //顶点类型、边类型
void Graph<Tv, Te>::BCC( int v, int& clock, Stack<int>& S ){ //assert: 0 <= v < n
   hca(v) = dTime(v) = ++clock; 
   status(v) = VStatus::DISCOVERED; 
   S.push (v); //v被发现并入栈

   for(int u = firstNbr(v); -1 < u; u = nextNbr(v, u)) { //枚举v的所有邻居u
        if (!exists(v, u)) {
            continue;
        }
        switch(status(u)){ //并视u的状态分别处理
            case VStatus::UNDISCOVERED:
                parent(u) = v; 
                type(v, u)= EType::TREE; 
                BCC(u, clock, S); //从顶点u处深入

                if (hca(u) < dTime(v)) //遍历返回后，若发现u（通过后向边）可指向v的真祖先
                    hca(v) = min(hca(v), hca(u)); //则v亦必如此
                else {//否则，以v为关节点（u以下即是一个BCC，且其中顶点此时正集中于栈S的顶部）
                    /*输出语句*/
                    Stack<int> temp; 
                    do { 
                        temp.push(S.pop()); 
                    } while ( u != temp.top() );

                    while (!temp.empty()) 
                        S.push(temp.pop());//将栈中的内容倒回去
                    /*输出语句*/
                    while (u != S.pop())
                        ; //弹出当前BCC中（除v外）的所有节点，可视需要做进一步处理
                    /*输出语句*/
                }

                break;
            case VStatus::DISCOVERED:
                type(v, u) = EType::BACKWARD; //标记(v, u)，并按照“越小越高”的准则
                if (u != parent(v)) 
                    hca(v) = ::min(hca(v), dTime(u)); //更新hca[v]
                break;
            default: //VISITED (digraphs only)
                type(v, u) = (dTime(v) < dTime(u))?EType::FORWARD:EType::CROSS;
                break;
        }
    }
    status(v) = VStatus::VISITED; //对v的访问结束
}

template <typename Tv, typename Te> template <typename PU>
void Graph<Tv, Te>::pfs( int s, PU prioUpdater ) {
   reset();
   for ( int v = s; v < s + n; v++ )
      if ( VStatus::UNDISCOVERED == status( v % n ) )
         PFS( v % n, prioUpdater );
}

template <typename Tv, typename Te> template <typename PU>
void Graph<Tv, Te>::PFS( int v, PU prioUpdater ) {
   priority( v ) = 0; status( v ) = VStatus::VISITED;
   for ( int k = 1 ; k < n ; k++ ) {
      for ( int u = firstNbr( v ); - 1 != u; u = nextNbr( v, u ) )
         prioUpdater( this, v, u );
      double shortest = priority(v);
      int next = -1;
      for ( int u = 0; u < n; u++ )
         if ( ( VStatus::UNDISCOVERED == status( u ) ) && ( (next == -1) || (priority( u ) < shortest) ) ) {
            shortest = priority( u ); next = u;
         }
      if (next == -1) break;
      v = next;
      status( v ) = VStatus::VISITED;
      if (parent(v) >= 0) type( parent( v ), v ) = EType::TREE;
   }
}

template <typename Tv, typename Te> //最短路径Dijkstra算法：适用于一般的有向图
void Graph<Tv, Te>::dijkstra( int s ) {
   reset(); priority( s ) = 0;
   for ( int i = 0; i < n; i++ ) {
      status( s ) = VStatus::VISITED;
      if ( -1 != parent( s ) ) type( parent( s ), s ) = EType::TREE;
      for ( int j = firstNbr( s ); -1 != j; j = nextNbr( s, j ) )
         if ( ( status( j ) == VStatus::UNDISCOVERED ) && ( priority( j ) > priority( s ) + weight( s, j ) ) )
            { priority( j ) = priority( s ) + weight( s, j ); parent( j ) = s; }
      double shortest = priority(s);
      int next = -1;
      for ( int j = 0; j < n; j++ )
         if ( ( status( j ) == VStatus::UNDISCOVERED ) && ( (next == -1) || (priority( j ) < shortest) ) ) {
            shortest = priority( j );
            next = j;
         }
      if (next == -1) break;
      s = next;
   }
}

template <typename Tv, typename Te>
void Graph<Tv, Te>::kruskal(bool flag){
    Vector<Edge<Te>> mst;
    Heap<Edge<Te>> pq;
    WeightedQuickUnionwithCompression uf(n);
    for(int i = 0; i < n; i++)
        for(int j = firstNbr(i); -1 < j; j = nextNbr(i, j)){
            Edge<Te> edge(Te(), weight(i, j), i, j);
            pq.insert(edge);
        }

    double weight = 0.00;
    while(!pq.empty() && mst.size() < n-1){
        Edge<Te> edge = pq.delMax();
        int v = edge.x;
        int w = edge.y;
        if(uf.connected(v, w)) continue;
        uf.unite(v, w);
        mst.insert(edge);

        weight += edge.weight;
        type(v, w) = EType::TREE;
        status(v) = VStatus::VISITED;
        status(w) = VStatus::VISITED; 
    }

    if(flag && observer){
        Vector<KruskalEdgeSummary<Tv, Te>> edges;
        for(int i = 0; i < mst.size(); i++){
            observer->onKruskalEdge(mst[i].x, mst[i].y, mst[i].weight);
            KruskalEdgeSummary<Tv, Te> summary{mst[i].x, mst[i].y};
            edges.insert(summary);
        }
        observer->onKruskalDone(weight, edges);
   }
}

template <typename Tv, typename Te> //Prim算法：无向连通图，各边表示为方向互逆、权重相等的一对边
void Graph<Tv, Te>::prim( int s ) { // s < n
   reset(); priority ( s ) = 0;
    for ( int i = 0; i < n; i++ ) {
      status( s ) = VStatus::VISITED;
      if ( -1 != parent( s ) ) type( parent( s ), s ) = EType::TREE;
      for ( int j = firstNbr( s ); -1 != j; j = nextNbr( s, j ) )
         if ( ( status( j ) == VStatus::UNDISCOVERED ) && ( priority( j ) > weight( s, j ) ) ) {
            priority( j ) = weight( s, j ); parent( j ) = s;
         }
      double shortest = priority(s);
      int next = -1;
      for ( int j = 0; j < n; j++ )
         if ( ( status( j ) == VStatus::UNDISCOVERED ) && ( (next == -1) || (priority( j ) < shortest) ) ) {
            shortest = priority( j );
            next = j;
         }
      if (next == -1) break;
      s = next;
   }
}

#endif
