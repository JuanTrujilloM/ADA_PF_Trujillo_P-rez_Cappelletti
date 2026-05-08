#pragma once
#include "parser.hpp"
#include <string>
#include <vector>

using namespace std;

struct BandwidthItem {
    string customerID;
    int tenure;
    double monthlyCharges;
    double totalCharges;

    // round(TotalCharges)
    int weight;

    // round(MonthlyCharges * 10)
    int value;
};

struct KnapsackResult {
    int optimalValue;
    vector<BandwidthItem> selected;

    // full table kept for backtracking
    vector<vector<int>> dp;
};

struct GreedyCounterexample {
    bool found;
    int capacity;
    long long evaluatedTriples;
    vector<BandwidthItem> triple;
    vector<BandwidthItem> greedySelection;
    int greedyValue;
    vector<BandwidthItem> optimalSelection;
    int optimalValue;
    
    // best attempt tracked even when greedy tied with DP
    vector<BandwidthItem> bestTriple;
    vector<BandwidthItem> bestGreedySelection;
    int bestGreedyValue;
    vector<BandwidthItem> bestOptimalSelection;
    int bestOptimalValue;
    int bestDifference;
};

// Builds the list of items to consider for the knapsack, applying the specified filters and transformations.
vector<BandwidthItem> buildActiveItems(const vector<Request>& sortedRequests,
                                       int maxItems = 50);

// Solves the 0-1 knapsack problem using dynamic programming and returns the optimal value and selected items.
KnapsackResult solveKnapsack01(const vector<BandwidthItem>& items, int capacity);

// Finds a counterexample of 3 items where the greedy by ratio v/w is suboptimal compared to the DP solution, within the given items and capacity.
GreedyCounterexample findGreedyCounterexample(const vector<BandwidthItem>& items,
                                              int capacity);

// Generates the bandwidth assignment report based on the sorted requests, writing the output to the specified path and using the given capacity for analysis.
void generateBandwidthReport(const vector<Request>& sortedRequests,
                             const string& outputPath = "results/asignacion_bw.txt",
                             int capacity = 500);
