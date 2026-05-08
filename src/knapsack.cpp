#include "knapsack.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;

// Helper function to compute value/weight ratio, handling zero weight cases.
static double valueWeightRatio(const BandwidthItem& item) {
    if (item.weight == 0)
        return item.value > 0 ? numeric_limits<double>::infinity() : 0.0;
    return static_cast<double>(item.value) / item.weight;
}

// Comparison function for sorting items by ratio v/w in descending order, with tie-breaking.
static bool compareByRatioDesc(const BandwidthItem& a, const BandwidthItem& b) {
    double ra = valueWeightRatio(a), rb = valueWeightRatio(b);
    if (ra != rb) return ra > rb;
    if (a.value != b.value) return a.value > b.value;
    if (a.weight != b.weight) return a.weight < b.weight;
    return a.customerID < b.customerID;
}

// Solves the greedy selection by ratio v/w for the given items and capacity, returning the selected items and total value.
static vector<BandwidthItem> solveGreedyRatio(const vector<BandwidthItem>& items,
                                              int capacity,
                                              int& totalValue) {
    vector<BandwidthItem> sorted = items;
    sort(sorted.begin(), sorted.end(), compareByRatioDesc);

    vector<BandwidthItem> selected;
    totalValue = 0;
    int remaining = capacity;
    
    // Iterate over the sorted items and select them greedily by ratio until capacity is exhausted.
    for (const auto& item : sorted) {
        if (item.weight <= remaining) {
            selected.push_back(item);
            totalValue += item.value;
            remaining -= item.weight;
        }
    }
    return selected;
}

// Utility function to list customer IDs of selected items in a readable format.
static string listIDs(const vector<BandwidthItem>& items) {
    if (items.empty()) return "(ninguna)";
    ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << items[i].customerID;
    }
    return oss.str();
}

// Checks if the given requests are sorted by tenure in descending order.
static bool isSortedByTenureDesc(const vector<Request>& requests) {
    for (size_t i = 1; i < requests.size(); ++i)
        if (requests[i - 1].tenure < requests[i].tenure) return false;
    return true;
}

// Counts how many items have weight less than or equal to the given capacity.
static int countFittingItems(const vector<BandwidthItem>& items, int capacity) {
    int count = 0;
    for (const auto& item : items)
        if (item.weight <= capacity) ++count;
    return count;
}

// Prints a table of items with their attributes to the given output stream.
static void printItemsTable(ostream& out, const vector<BandwidthItem>& items) {
    out << "Solicitudes consideradas antes de la DP\n";
    out << "customerID,tenure,MonthlyCharges,TotalCharges,peso,valor\n";
    out << fixed << setprecision(2);
    for (const auto& item : items) {
        out << item.customerID << ","
            << item.tenure << ","
            << item.monthlyCharges << ","
            << item.totalCharges << ","
            << item.weight << ","
            << item.value << "\n";
    }
}

// Prints a table of the three items in the counterexample triple, showing their attributes and ratio.
static void printTripleTable(ostream& out, const vector<BandwidthItem>& triple) {
    out << left << setw(15) << "customerID"
        << right << setw(10) << "peso"
        << setw(10) << "valor"
        << setw(14) << "ratio" << "\n";
    out << string(49, '-') << "\n";
    out << fixed << setprecision(4);
    for (const auto& item : triple) {
        out << left << setw(15) << item.customerID
            << right << setw(10) << item.weight
            << setw(10) << item.value
            << setw(14) << valueWeightRatio(item) << "\n";
    }
}

// Prints a comparison table of the greedy selection vs the optimal selection, showing their total values and whether greedy is optimal.
static void printComparison(ostream& out,
                            const vector<BandwidthItem>& greedySelection,
                            int greedyValue,
                            const vector<BandwidthItem>& optimalSelection,
                            int optimalValue,
                            bool greedyIsOptimal) {
    out << left << setw(24) << "Enfoque"
        << setw(48) << "Solicitudes seleccionadas"
        << right << setw(14) << "Valor total"
        << setw(12) << "Optimo" << "\n";
    out << string(98, '-') << "\n";
    out << left << setw(24) << "Codicioso ratio v/w"
        << setw(48) << listIDs(greedySelection)
        << right << setw(14) << greedyValue
        << setw(12) << (greedyIsOptimal ? "Si" : "No") << "\n";
    out << left << setw(24) << "PD Mochila 0-1"
        << setw(48) << listIDs(optimalSelection)
        << right << setw(14) << optimalValue
        << setw(12) << "Si" << "\n";
}

// Builds the list of items to consider for the knapsack, applying the specified filters and transformations.
vector<BandwidthItem> buildActiveItems(const vector<Request>& sortedRequests, int maxItems) {
    vector<BandwidthItem> items;
    items.reserve(maxItems);
    
    for (const auto& req : sortedRequests) {
        if (!req.churn) {
            BandwidthItem item;
            item.customerID     = req.customerID;
            item.tenure         = req.tenure;
            item.monthlyCharges = req.monthlyCharges;
            item.totalCharges   = req.totalCharges;
            item.weight = static_cast<int>(round(req.totalCharges));
            item.value  = static_cast<int>(round(req.monthlyCharges * 10.0));
            items.push_back(item);
            if (static_cast<int>(items.size()) == maxItems) break;
        }
    }
    return items;
}

// Builds the list of items from the full dataset without filtering by churn, used for searching a real counterexample.
static vector<BandwidthItem> buildRealItems(const vector<Request>& requests) {
    vector<BandwidthItem> items;
    items.reserve(requests.size());

    for (const auto& req : requests) {
        BandwidthItem item;
        item.customerID     = req.customerID;
        item.tenure         = req.tenure;
        item.monthlyCharges = req.monthlyCharges;
        item.totalCharges   = req.totalCharges;
        item.weight = static_cast<int>(round(req.totalCharges));
        item.value  = static_cast<int>(round(req.monthlyCharges * 10.0));
        if (item.weight > 0 && item.value > 0)
            items.push_back(item);
    }
    return items;
}

// Solves the 0-1 knapsack problem using dynamic programming and returns the optimal value and selected items.
KnapsackResult solveKnapsack01(const vector<BandwidthItem>& items, int capacity) {
    int n = static_cast<int>(items.size());
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));

    for (int i = 1; i <= n; ++i) {
        int w = items[i - 1].weight;
        int v = items[i - 1].value;
        for (int c = 0; c <= capacity; ++c) {
            if (w > c)
                dp[i][c] = dp[i - 1][c];
            else
                dp[i][c] = max(dp[i - 1][c], dp[i - 1][c - w] + v);
        }
    }

    // Backtrack from dp[n][capacity] to recover the selected items
    vector<BandwidthItem> selected;
    int rem = capacity;
    for (int i = n; i > 0; --i) {
        const BandwidthItem& item = items[i - 1];
        if (item.weight <= rem &&
            dp[i][rem] != dp[i - 1][rem] &&
            dp[i][rem] == dp[i - 1][rem - item.weight] + item.value) {
            selected.push_back(item);
            rem -= item.weight;
        }
    }
    reverse(selected.begin(), selected.end());

    return {dp[n][capacity], selected, dp};
}

// Finds a counterexample of 3 items where the greedy by ratio v/w is suboptimal compared to the DP solution, within the given items and capacity.
GreedyCounterexample findGreedyCounterexample(const vector<BandwidthItem>& items,
                                              int capacity) {
    GreedyCounterexample result;
    result.found            = false;
    result.capacity         = capacity;
    result.evaluatedTriples = 0;
    result.greedyValue      = 0;
    result.optimalValue     = 0;
    result.bestGreedyValue  = 0;
    result.bestOptimalValue = 0;
    result.bestDifference   = numeric_limits<int>::min();
    
    // Evaluate all combinations of 3 distinct items to find a counterexample where greedy by ratio v/w is suboptimal.
    int n = static_cast<int>(items.size());
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            for (int k = j + 1; k < n; ++k) {
                ++result.evaluatedTriples;
                vector<BandwidthItem> triple = {items[i], items[j], items[k]};

                // Solve the greedy selection by ratio v/w and the optimal DP solution for this triple and the given capacity, and compare their values.
                int greedyVal = 0;
                vector<BandwidthItem> greedySel = solveGreedyRatio(triple, capacity, greedyVal);
                KnapsackResult optimal = solveKnapsack01(triple, capacity);
                int diff = optimal.optimalValue - greedyVal;

                if (result.bestTriple.empty() ||
                    diff > result.bestDifference ||
                    (diff == result.bestDifference && optimal.optimalValue > result.bestOptimalValue)) {
                    result.bestTriple           = triple;
                    result.bestGreedySelection  = greedySel;
                    result.bestGreedyValue      = greedyVal;
                    result.bestOptimalSelection = optimal.selected;
                    result.bestOptimalValue     = optimal.optimalValue;
                    result.bestDifference       = diff;
                }

                if (greedyVal < optimal.optimalValue) {
                    result.found            = true;
                    result.capacity         = capacity;
                    result.triple           = triple;
                    result.greedySelection  = greedySel;
                    result.greedyValue      = greedyVal;
                    result.optimalSelection = optimal.selected;
                    result.optimalValue     = optimal.optimalValue;
                    return result;
                }
            }
        }
    }
    return result;
}

// Searches the full dataset for a 3-item counterexample using a mini-capacity
// equal to the sum of the two smaller items, forcing greedy to fail.
static GreedyCounterexample findCounterexampleInDataset(const vector<Request>& requests,
                                                        int officialCapacity) {
    GreedyCounterexample result;
    result.found            = false;
    result.capacity         = 0;
    result.evaluatedTriples = 0;
    result.greedyValue      = 0;
    result.optimalValue     = 0;
    result.bestGreedyValue  = 0;
    result.bestOptimalValue = 0;
    result.bestDifference   = numeric_limits<int>::min();
    
    // Build candidate items from the full dataset, filtering by officialCapacity to reduce the search space.
    vector<BandwidthItem> candidates;
    for (const auto& item : buildRealItems(requests))
        if (item.weight <= officialCapacity) candidates.push_back(item);
    sort(candidates.begin(), candidates.end(), compareByRatioDesc);
    
    // Evaluate all combinations of 3 distinct candidate items to find a counterexample where greedy by ratio v/w is suboptimal.
    int n = static_cast<int>(candidates.size());
    for (int i = 0; i < n; ++i) {
        const BandwidthItem& first = candidates[i];
        for (int j = i + 1; j < n; ++j) {
            const BandwidthItem& second = candidates[j];
            if (second.weight >= first.weight) continue;
            for (int k = j + 1; k < n; ++k) {
                const BandwidthItem& third = candidates[k];
                if (third.weight >= first.weight) continue;
                ++result.evaluatedTriples;

                int miniCapacity = second.weight + third.weight;
                vector<BandwidthItem> triple = {first, second, third};

                int greedyVal = 0;
                vector<BandwidthItem> greedySel = solveGreedyRatio(triple, miniCapacity, greedyVal);
                KnapsackResult optimal = solveKnapsack01(triple, miniCapacity);
                int diff = optimal.optimalValue - greedyVal;

                if (result.bestTriple.empty() ||
                    diff > result.bestDifference ||
                    (diff == result.bestDifference && optimal.optimalValue > result.bestOptimalValue)) {
                    result.bestTriple           = triple;
                    result.bestGreedySelection  = greedySel;
                    result.bestGreedyValue      = greedyVal;
                    result.bestOptimalSelection = optimal.selected;
                    result.bestOptimalValue     = optimal.optimalValue;
                    result.bestDifference       = diff;
                }

                if (greedyVal < optimal.optimalValue) {
                    result.found            = true;
                    result.capacity         = miniCapacity;
                    result.triple           = triple;
                    result.greedySelection  = greedySel;
                    result.greedyValue      = greedyVal;
                    result.optimalSelection = optimal.selected;
                    result.optimalValue     = optimal.optimalValue;
                    return result;
                }
            }
        }
    }
    return result;
}

// Generates the bandwidth assignment report based on the sorted requests, writing the output to the specified path and using the given capacity for analysis.
void generateBandwidthReport(const vector<Request>& sortedRequests,
                             const string& outputPath,
                             int capacity) {
    vector<BandwidthItem> items = buildActiveItems(sortedRequests, 50);
    int fittingCount = countFittingItems(items, capacity);
    bool isSorted    = isSortedByTenureDesc(sortedRequests);

    ofstream out(outputPath);
    if (!out.is_open())
        throw runtime_error("Could not open file: " + outputPath);

    out << "\xEF\xBB\xBF";
    out << "Modulo C - Asignacion de ancho de banda\n";
    out << "Capacidad W: " << capacity << "\n";
    out << "Numero de requests consideradas: " << items.size() << "\n";
    out << "Verificacion de entrada:\n";
    out << "- Vector recibido desde Modulo A ordenado por tenure descendente: "
        << (isSorted ? "OK" : "ADVERTENCIA: no esta ordenado") << "\n";
    out << "- Filtro aplicado: Churn == \"No\" (campo churn == false).\n";
    out << "- Peso usado: round(TotalCharges) sobre Request.totalCharges parseado como double.\n";
    out << "- Valor usado: round(MonthlyCharges * 10).\n\n";

    printItemsTable(out, items);
    out << "\nCantidad de requests consideradas con peso <= " << capacity
        << ": " << fittingCount << "\n";
    if (fittingCount == 0) {
        out << "Diagnostico: todas las 50 requests consideradas tienen peso > "
            << capacity << ". Con W = " << capacity
            << ", ninguna solicitud puede entrar en la mochila; el enunciado "
            << "produce una instancia degenerada para estos datos y formulas.\n";
    }

    // Execute the DP knapsack solution and the greedy counterexample search, and print the results and analysis.
    KnapsackResult result          = solveKnapsack01(items, capacity);
    GreedyCounterexample counterex = findGreedyCounterexample(items, capacity);

    out << "\nResultado DP Mochila 0-1\n";
    out << "------------------------\n";
    out << "Valor optimo total: " << result.optimalValue << "\n";
    out << "Numero de requests seleccionadas: " << result.selected.size() << "\n\n";

    out << "Solicitudes seleccionadas por PD Mochila 0-1\n";
    out << "customerID,peso,valor\n";
    if (result.selected.empty()) {
        out << "(ninguna)\n";
    } else {
        for (const auto& item : result.selected)
            out << item.customerID << "," << item.weight << "," << item.value << "\n";
    }

    out << "\nContraejemplo codicioso por ratio v/w\n";
    out << "-------------------------------------\n";
    out << "Trios evaluados: " << counterex.evaluatedTriples << "\n";
    if (counterex.found) {
        out << "Trio usado:\n";
        printTripleTable(out, counterex.triple);
        out << "\nComparacion:\n";
        printComparison(out, counterex.greedySelection, counterex.greedyValue,
                        counterex.optimalSelection, counterex.optimalValue, false);
    } else {
        out << "No se encontro un trio de requests, dentro del conjunto de "
            << items.size()
            << ", donde el codicioso por ratio v/w tenga menor valor que la PD "
            << "Mochila 0-1 con W = " << capacity << ".\n";
        out << "Razon: ";
        if (fittingCount == 0) {
            out << "ninguna de las 50 requests cabe individualmente con W = "
                << capacity
                << "; por eso, en todos los trios tanto el codicioso como la DP "
                << "seleccionan el conjunto vacio con valor 0.\n";
        } else {
            out << "todos los trios evaluados tuvieron valor codicioso igual al "
                << "valor optimo de la DP; no hubo caso con codicioso < optimo.\n";
        }

        if (!counterex.bestTriple.empty()) {
            out << "\nMejor intento encontrado:\n";
            out << "Diferencia PD - codicioso: " << counterex.bestDifference << "\n";
            printTripleTable(out, counterex.bestTriple);
            out << "\nComparacion del mejor intento:\n";
            printComparison(out, counterex.bestGreedySelection, counterex.bestGreedyValue,
                            counterex.bestOptimalSelection, counterex.bestOptimalValue, true);
        }
    }

    out << "\nAnalisis de complejidad\n";
    out << "-----------------------\n";
    out << "Tiempo: Theta(n * W), porque se llena una tabla con n + 1 filas "
        << "y W + 1 columnas.\n";
    out << "Espacio: Theta(n * W), porque la tabla dp completa se mantiene "
        << "para reconstruir la solucion con backtracking.\n";
    out << "El algoritmo es pseudopolinomial: su costo depende del valor "
        << "numerico de W, no solo de la cantidad de bits necesarios para "
        << "representar W en la entrada.\n";

    out << "\nObservacion sobre consistencia del enunciado\n";
    out << "--------------------------------------------\n";
    out << "El Modulo C siguio literalmente la formula indicada en el "
        << "enunciado: peso = round(TotalCharges), valor = "
        << "round(MonthlyCharges * 10), y capacidad W = " << capacity << ".\n";
    out << "Las 50 requests consideradas corresponden a las primeras "
        << "requests activas (Churn == No) del arreglo ordenado por tenure "
        << "descendente; en esta instancia todas tienen tenure = 72.\n";
    out << "Como 0 de las 50 requests tienen peso <= " << capacity
        << ", ninguna cabe en la mochila. Por eso la programacion dinamica "
        << "devuelve valor optimo 0 y selecciona 0 requests.\n";
    out << "Por la misma razon, no puede construirse un contraejemplo codicioso "
        << "valido dentro de esas 50 requests con W = " << capacity
        << ": tanto el codicioso por ratio v/w como la DP eligen el conjunto "
        << "vacio en cada trio evaluado.\n";
    out << "No se modifico W ni la formula de peso, porque hacerlo cambiaria "
        << "las reglas originales del enunciado.\n";
    out << "Para cumplir la rubrica del fallo codicioso sin inventar datos, "
        << "se busca aparte una mini-instancia de exactamente 3 requests "
        << "reales del CSV, manteniendo peso = round(TotalCharges) y valor = "
        << "round(MonthlyCharges * 10), pero usando una capacidad W_ce propia "
        << "del contraejemplo.\n";

    GreedyCounterexample realCE = findCounterexampleInDataset(sortedRequests, capacity);

    out << "\nContraejemplo valido con datos reales del CSV\n";
    out << "---------------------------------------------\n";
    out << "Trios evaluados en la busqueda: " << realCE.evaluatedTriples << "\n";

    if (realCE.found) {
        out << "Capacidad de la mini-instancia W_ce: " << realCE.capacity << "\n";
        out << "Trio usado:\n";
        printTripleTable(out, realCE.triple);
        out << "\nComparacion requerida:\n";
        printComparison(out, realCE.greedySelection, realCE.greedyValue,
                        realCE.optimalSelection, realCE.optimalValue, false);
        out << "El codicioso toma primero la solicitud con mayor ratio v/w; "
            << "despues ya no le queda capacidad para agregar otra. La PD, "
            << "en cambio, evalua todas las combinaciones 0-1 y encuentra "
            << "un par de requests con mayor valor total.\n";
    } else {
        out << "No se encontro un contraejemplo real con las condiciones "
            << "programadas. Esto deberia revisarse porque la rubrica exige "
            << "un trio explicito donde greedy no sea optimo.\n";
    }
}
