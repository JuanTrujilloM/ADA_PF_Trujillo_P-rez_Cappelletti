#pragma once
#include "parser.hpp"

// Returns the index of a request with tenure == k in a descending-sorted array. Returns -1 if none exists.
int busquedaBinaria(const vector<Solicitud>& arr, int left, int right, int k);