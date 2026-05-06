#pragma once
#include <vector>
#include <string>
#include "parser.hpp"

using namespace std;

struct Edge {
    int u, v;
    int cost;
};

class UnionFind {
public:
    vector<int> parent;
    vector<int> rank;

    UnionFind(int n);
    int find(int x);
    bool unite(int x, int y);
};

vector<Edge> buildGraph(const vector<Solicitud>& solicitudes,
                        vector<double>& groupAvg);