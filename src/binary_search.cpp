#include "binary_search.hpp"

using namespace std;

// Returns the index of an element with tenure == k in a descending-sorted array,
// or -1 if no such element exists.
int busquedaBinaria(const vector<Solicitud>& arr, int left, int right, int k) {
    if (left > right)
        return -1;

    int mid = (left + right) / 2;

    if (arr[mid].tenure == k) {
        return mid;
    } else if (arr[mid].tenure > k) {
        return busquedaBinaria(arr, mid + 1, right, k);
    } else {
        return busquedaBinaria(arr, left, mid - 1, k);
    }
}
