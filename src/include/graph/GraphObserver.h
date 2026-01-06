#pragma once

#include "Vector.h"

template<typename Tv, typename Te>
struct KruskalEdgeSummary {
    int u;
    int v;
};

// 观察者接口，默认不做任何输出
template<typename Tv, typename Te>
struct GraphObserver {
    virtual ~GraphObserver() = default;
    virtual void onSCCComponent(const Vector<int>& nodes) { (void)nodes; }
    virtual void onKruskalEdge(int u, int v, double w) { (void)u; (void)v; (void)w; }
    virtual void onKruskalDone(double totalWeight, const Vector<KruskalEdgeSummary<Tv, Te>>& edges) {
        (void)totalWeight;
        (void)edges;
    }
};

template<typename Tv, typename Te>
struct NoopGraphObserver : GraphObserver<Tv, Te> {};
