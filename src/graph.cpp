#include "graph.hpp"
#include <cmath>
#include <numeric>

UnionFind::UnionFind(int n) : parent(n), rank(n, 0) {
    iota(parent.begin(), parent.end(), 0);
}

int UnionFind::find(int x) {
    if (parent[x] != x)
        parent[x] = find(parent[x]);
    return parent[x];
}

bool UnionFind::unite(int x, int y) {
    int rx = find(x), ry = find(y);
    if (rx == ry) return false;

    if (rank[rx] < rank[ry]) swap(rx, ry);
    parent[ry] = rx;
    if (rank[rx] == rank[ry]) rank[rx]++;
    return true;
}

static double roundTo2(double x) {
    return round(x * 100.0) / 100.0;
}

vector<Edge> buildGraph(const vector<Solicitud>& solicitudes,
                        vector<double>& groupAvg) {
    const int N = 20;
    groupAvg.assign(N, 0.0);

    vector<double> sum(N, 0.0);
    vector<int>    cnt(N, 0);

    for (int i = 0; i < (int)solicitudes.size(); i++) {
        int g = i % N;
        sum[g] += solicitudes[i].monthlyCharges;
        cnt[g]++;
    }

    for (int k = 0; k < N; k++) {
        groupAvg[k] = roundTo2(sum[k] / cnt[k]);
    }

    vector<Edge> edges;
    edges.reserve(N * (N - 1) / 2);

    for (int u = 0; u < N; u++) {
        for (int v = u + 1; v < N; v++) {
            int c = (int)floor(groupAvg[u] + groupAvg[v]);
            edges.push_back({u, v, c});
        }
    }

    return edges;
}