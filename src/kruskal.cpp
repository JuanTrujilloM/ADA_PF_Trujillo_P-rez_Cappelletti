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

    Edge firstEdge = mst.mstEdges[0];

    out << "Lema del ciclo: Para cualquier corte (S, V-S) de un grafo conexo\n";
    out << "ponderado, la arista de menor peso que cruza el corte pertenece a\n";
    out << "algun MST.\n\n";
 
    out << "Aplicacion al grafo generado:\n\n";
 
    out << "  Arista concreta: (" << firstEdge.u << ", " << firstEdge.v
        << ") con costo " << firstEdge.cost << ".\n\n";
 
    out << "  Consideremos el corte S = {" << firstEdge.u
        << "}, V-S = {todos los demas nodos}.\n";
    out << "  La arista (" << firstEdge.u << ", " << firstEdge.v
        << ") es la de menor peso que cruza este corte,\n";
    out << "  ya que cualquier otra arista incidente al nodo " << firstEdge.u
        << " tiene costo >= " << firstEdge.cost << ".\n\n";
 
    out << "  Demostracion por contradiccion:\n";
    out << "  Supongamos que existe un MST T* que NO incluye la arista ("
        << firstEdge.u << ", " << firstEdge.v << ").\n";
    out << "  Entonces T* usa otra arista (u', v') para conectar el nodo "
        << firstEdge.u << " al resto\n";
    out << "  del arbol, con costo c(u', v') >= " << firstEdge.cost << ".\n";
    out << "  Al anadir (" << firstEdge.u << ", " << firstEdge.v
        << ") a T* se forma un ciclo C.\n";
    out << "  Al remover (u', v') de C se obtiene un nuevo arbol T' con:\n";
    out << "    peso(T') = peso(T*) - c(u', v') + " << firstEdge.cost
        << " <= peso(T*)\n";
    out << "  Esto contradice que T* sea estrictamente mejor que T'.\n";
    out << "  Por lo tanto, la arista (" << firstEdge.u << ", " << firstEdge.v
        << ") pertenece a algun MST.\n\n";
 
    out << "  Conclusion: Kruskal, al seleccionar en cada paso la arista de menor\n";
    out << "  costo que no forme ciclo, aplica repetidamente el Lema del ciclo,\n";
    out << "  garantizando que el resultado es un MST optimo global.\n";

    out.close();
}