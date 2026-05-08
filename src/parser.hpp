#pragma once
#include <string>
#include <vector>

using namespace std;

// Represents a customer request (solicitud) with relevant fields.
struct Request {
    string customerID;
    int tenure;
    double monthlyCharges;
    double totalCharges;
    bool churn;
};

// Reads the CSV and returns the vector of requests. 
// nullCount: number of records with blank TotalCharges (tenure = 0).
vector<Request> parseCSV(const string& path, int& nullCount);
