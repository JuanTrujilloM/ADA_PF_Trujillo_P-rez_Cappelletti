#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

// Helper function to split a CSV line into fields
static vector<string> splitCSV(const string& line) {
    vector<string> fields;
    stringstream ss(line);
    string field;
    while (getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

// Reads the CSV and returns the vector of requests.
vector<Request> parseCSV(const string& path, int& nullCount) {
    ifstream file(path);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + path);
    }

    vector<Request> requests;
    nullCount = 0;
    string line;

    // skip header
    getline(file, line);

    // read each line
    while (getline(file, line)) {
        if (line.empty()) continue;

        vector<string> fields = splitCSV(line);
        if (fields.size() < 21) continue;

        Request s;
        s.customerID = fields[0];
        s.tenure = stoi(fields[5]);
        s.monthlyCharges = stod(fields[18]);

        // TotalCharges can be a blank space when tenure = 0
        string tc = fields[19];
        bool onlySpaces = true;
        for (char c : tc) {
            if (c != ' ') {
                onlySpaces = false;
                break;
            }
        }
        // Replace blank TotalCharges with 0.0 and count them
        if (tc.empty() || onlySpaces) {
            s.totalCharges = 0.0;
            nullCount++;
        } else {
            s.totalCharges = stod(tc);
        }

        s.churn = (fields[20] == "Yes");

        requests.push_back(s);
    }

    return requests;
}
