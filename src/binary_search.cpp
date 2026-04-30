#include "binary_search.hpp"

using namespace std;

// Returns the index of the rightmost element with tenure >= k, or -1 if not found.
int busquedaBinaria(const vector<Solicitud>& arr, int left, int right, int k) {
    if (left > right)
        return -1;

    int mid = (left + right) / 2;

    if (arr[mid].tenure >= k) {
        // mid qualifies, but there may be another further right
        int result = busquedaBinaria(arr, mid + 1, right, k);
        if (result != -1) {
            return result;
        } else {
            return mid;
        }
    } else {
        // mid does not qualify, so search left where tenures are larger
        return busquedaBinaria(arr, left, mid - 1, k);
    }
}
