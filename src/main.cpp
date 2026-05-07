#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include "parser.hpp"
#include "mergesort.hpp"
#include "binary_search.hpp"
#include "kruskal.hpp"
#include "knapsack.hpp"

using namespace std;

static double measureTime(vector<Solicitud> sample, int n) {
    vector<Solicitud> sub(sample.begin(), sample.begin() + n);
    auto start = chrono::high_resolution_clock::now();
    mergeSort(sub, 0, sub.size() - 1);
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(end - start).count();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./ada_pf data/WA_Fn-UseC_-Telco-Customer-Churn.csv\n";
        return 1;
    }

    // Module A
    int nullCount = 0;
    vector<Solicitud> requests = parsearCSV(argv[1], nullCount);

    cout << "MODULE A\n";
    cout << "Records loaded: " << requests.size() << "\n";
    cout << "Records with null TotalCharges: " << nullCount << "\n";

    mergeSort(requests, 0, requests.size() - 1);

    ofstream csvOut("results/solicitudes_ordenadas.csv");
    csvOut << "customerID,tenure,monthlyCharges,totalCharges,churn\n";
    for (const auto& s : requests) {
        csvOut << s.customerID << ","
               << s.tenure << ","
               << fixed << setprecision(2) << s.monthlyCharges << ","
               << s.totalCharges << ",";
        csvOut << (s.churn ? "Yes" : "No") << "\n";
    }
    csvOut.close();
    cout << "File generated: results/solicitudes_ordenadas.csv\n";

    // Binary search
    int queries[] = {72, 60, 45, 30, 12};
    string names[] = {"Q_A01", "Q_A02", "Q_A03", "Q_A04", "Q_A05"};

    ofstream busOut("results/busquedas_A.txt");
    busOut << "Binary search results\n\n";
    cout << "\nBinary searches:\n";

    for (int i = 0; i < 5; i++) {
        int idx = busquedaBinaria(requests, 0, requests.size() - 1, queries[i]);
        string result = (idx != -1) ? requests[idx].customerID : "Not found";
        cout << names[i] << " (k=" << queries[i] << "): " << result << "\n";
        busOut << names[i] << " | k=" << queries[i] << " | customerID=" << result << "\n";
    }
    busOut.close();
    cout << "File generated: results/busquedas_A.txt\n";

    // Empirical timing analysis
    cout << "\nMergeSort timing:\n";
    cout << left << setw(12) << "n" << setw(15) << "time (ms)" << "\n";
    cout << string(27, '-') << "\n";

    int sizes[] = {1000, 3500, 7043};
    for (int n : sizes) {
        double t = measureTime(requests, n);
        cout << setw(12) << n << setw(15) << fixed << setprecision(2) << t << "\n";
    }

    // Module B
    vector<double> groupAvg;
    vector<Edge> edges = buildGraph(requests, groupAvg);
    MSTResult mst = kruskal(20, edges);
    writeMSTReport("results/mst_red.txt", edges, groupAvg, mst);

    cout << "\nMODULE B\n";
    cout << "Graph: 20 nodes, " << edges.size() << " edges\n";
    cout << "MST total weight: " << mst.totalWeight << "\n";
    cout << "File generated: results/mst_red.txt\n";
 
    // Module C
    generarReporteAsignacionBW(requests);
    cout << "File generated: results/asignacion_bw.txt\n";

    return 0;
}
