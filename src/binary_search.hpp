#pragma once
#include "parser.hpp"

// Returns the index of the last request with tenure >= k. Returns -1 if none exists.
int busquedaBinaria(const vector<Solicitud>& arr, int left, int right, int k);