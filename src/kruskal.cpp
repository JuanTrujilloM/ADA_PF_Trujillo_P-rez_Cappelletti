#include "kruskal.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <iostream>

MSTResult kruskal(int numNodes, vector<Edge> edges) {
    sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.cost < b.cost;
    });

    UnionFind uf(numNodes);
    MSTResult result;
    result.totalWeight = 0;

    for (const auto& e : edges) {
        if (uf.unite(e.u, e.v)) {
            result.mstEdges.push_back(e);
            result.totalWeight += e.cost;
            if ((int)result.mstEdges.size() == numNodes - 1) break;
        }
    }

    return result;
}

void writeMSTReport(const string& path,
                    const vector<Edge>& allEdges,
                    const vector<double>& groupAvg,
                    const MSTResult& mst) {
    ofstream out(path);
    if (!out.is_open()) {
        cerr << "Error: could not open " << path << endl;
        return;
    }

    out << fixed << setprecision(2);

    int numNodes = (int)groupAvg.size();
    int numEdges = (int)allEdges.size();
    double avgCost = 0.0;
    for (const auto& e : allEdges) avgCost += e.cost;
    avgCost /= numEdges;

    out << "\n";
    out << "  MODULO B - Red de minimo costo\n";
    out << "\n\n";

    out << " Estadisticas del grafo \n";
    out << "Numero de nodos:           " << numNodes << "\n";
    out << "Numero de aristas:         " << numEdges << "\n";
    out << "Costo promedio de arista:  " << avgCost << "\n\n";

    out << " Promedios MonthlyCharges por grupo \n";
    for (int k = 0; k < numNodes; k++) {
        out << "  Grupo " << setw(2) << k << ": " << groupAvg[k] << "\n";
    }
    out << "\n";

    out << " Arbol de expansion minima (MST) \n";
    out << "Aristas del MST (" << mst.mstEdges.size() << " aristas):\n";
    out << setw(8) << "Nodo u" << setw(8) << "Nodo v" << setw(10) << "Costo" << "\n";
    out << "\n";
    for (const auto& e : mst.mstEdges) {
        out << setw(8) << e.u << setw(8) << e.v << setw(10) << e.cost << "\n";
    }
    out << "\nPeso total del MST: " << mst.totalWeight << "\n";

    out.close();
}