#pragma once
#include "graph.hpp"
#include <string>

struct MSTResult {
    vector<Edge> mstEdges;
    int totalWeight;
};

MSTResult kruskal(int numNodes, vector<Edge> edges);

void writeMSTReport(const string& path,
                    const vector<Edge>& allEdges,
                    const vector<double>& groupAvg,
                    const MSTResult& mst);