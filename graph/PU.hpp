#pragma once
#include "./Graph.hpp"

template <typename Tv, typename Te> struct BfsPU { //针对BFS算法的顶点优先级更新器
   virtual void operator() ( Graph<Tv, Te>* g, int uk, int v ) {
      if ( g->status ( v ) == VStatus::UNDISCOVERED ) //对于uk每一尚未被发现的邻接顶点v
         if ( g->priority ( v ) > g->priority ( uk ) + 1 ) { //将其到起点的距离作为优先级数
            g->priority ( v ) = g->priority ( uk ) + 1; //更新优先级（数）
            g->parent ( v ) = uk; //更新父节点
         } //如此效果等同于，先被发现者优先
   }
};

template <typename Tv, typename Te> struct DfsPU { //针对DFS算法的顶点优先级更新器
   virtual void operator() ( Graph<Tv, Te>* g, int uk, int v ) {
      if ( g->status ( v ) == VStatus::UNDISCOVERED ) //对于uk每一尚未被发现的邻接顶点v
         if ( g->priority ( v ) > g->priority ( uk ) - 1 ) { //将其到起点距离的负数作为优先级数
            g->priority ( v ) = g->priority ( uk ) - 1; //更新优先级（数）
            g->parent ( v ) = uk; //更新父节点
            return; //注意：与BfsPU()不同，这里只要有一个邻接顶点可更新，即可立即返回
         } //如此效果等同于，后被发现者优先
   }
};

template <typename Tv, typename Te> struct DijkstraPU { //针对Dijkstra算法的顶点优先级更新器
   virtual void operator() ( Graph<Tv, Te>* g, int uk, int v ) {
      if ( VStatus::UNDISCOVERED == g->status ( v ) ) //对于uk每一尚未被发现的邻接顶点v，按Dijkstra策略
         if ( g->priority ( v ) > g->priority ( uk ) + g->weight ( uk, v ) ) { //做松弛
            g->priority ( v ) = g->priority ( uk ) + g->weight ( uk, v ); //更新优先级（数）
            g->parent ( v ) = uk; //并同时更新父节点
         }
   }
};

template <typename Tv, typename Te> struct PrimPU {
   virtual void operator()(Graph<Tv, Te>* g, int uk, int v) {
      if ( VStatus::UNDISCOVERED == g->status ( v ) ) //对于uk每一尚未被发现的邻接顶点v，按Dijkstra策略
         if ( g->priority ( v ) > g->weight ( uk, v ) ) { //做松弛
            g->priority ( v ) = g->weight ( uk, v ); //更新优先级（数）
            g->parent ( v ) = uk; //并同时更新父节点
         }
   }
};