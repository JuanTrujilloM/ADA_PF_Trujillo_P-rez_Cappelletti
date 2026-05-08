#include "mergesort.hpp"

using namespace std;

// Merges two sorted subarrays of arr. First subarray is arr[left..mid]
static void merge(vector<Request>& arr, int left, int mid, int right) {

    // arrayOne = arr[left..mid]
    vector<Request> arrayOne(arr.begin() + left, arr.begin() + mid + 1);
    
    // arrayTwo = arr[mid+1..right]
    vector<Request> arrayTwo(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0;
    int j = 0;
    int k = left;

    // while both arrays have elements
    while (i < arrayOne.size() && j < arrayTwo.size()) {
        // take the larger first (descending)
        if (arrayOne[i].tenure > arrayTwo[j].tenure) {
            arr[k] = arrayOne[i];
            i++;
        } else {
            arr[k] = arrayTwo[j];
            j++;
        }
        k++;
    }

    // while arrayOne has elements
    while (i < arrayOne.size()) {
        arr[k] = arrayOne[i];
        i++;
        k++;
    }

    // while arrayTwo has elements
    while (j < arrayTwo.size()) {
        arr[k] = arrayTwo[j];
        j++;
        k++;
    }
}

// Sorts arr[left..right] using merge()
void mergeSort(vector<Request>& arr, int left, int right) {
    // base case: single element
    if (left >= right)
        return;

    int mid = (left + right) / 2;

    // arrayOne = mergeSort(arrayOne)
    mergeSort(arr, left, mid);
    
    // arrayTwo = mergeSort(arrayTwo)
    mergeSort(arr, mid + 1, right);

    // return merge(arrayOne, arrayTwo)
    merge(arr, left, mid, right);   
}
