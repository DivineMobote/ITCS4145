#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;
using namespace chrono;

void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; ++i)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;


    while (i < n1 && j < n2) {
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }

    
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}


void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: ./mergesort <size>\n";
        return 1;
    }

    int n = atoi(argv[1]);
    vector<int> arr(n);

    
    srand(time(0));

    
    for (int i = 0; i < n; ++i)
        arr[i] = rand();

    auto start = high_resolution_clock::now();
    mergeSort(arr, 0, n - 1);
    auto end = high_resolution_clock::now();

    duration<double> elapsed = end - start;

    // Output format: size,time
    cout << n << "," << elapsed.count() << endl;

    return 0;
}
