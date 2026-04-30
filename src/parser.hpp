#pragma once
#include <string>
#include <vector>

using namespace std;

struct Solicitud {
    string customerID;
    int tenure;
    double monthlyCharges;
    double totalCharges;
    bool churn;
};

// Reads the CSV and returns the vector of requests. 
// nullCount: number of records with blank TotalCharges (tenure = 0).
vector<Solicitud> parsearCSV(const string& path, int& nullCount);
