#include <stdio.h>
#include <stdlib.h>

int bsearch_lower_bound(int arr[], int n, int x) {
    int l = 0, r = n - 1;
    int res = -1;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (arr[mid] == x) {
            res = mid;
            r = mid - 1;
        } else if (arr[mid] < x) {
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }
    return res;
}

int main() {
    long long Input = { 0, 2, 5, 7, 8 };
    int n = 5;

    bsearch_lower_bound(Input, n, 0); // retorna 0
    bsearch_lower_bound(Input, n, 1); // retorna 1
    bsearch_lower_bound(Input, n, 2); // retorna 1
    bsearch_lower_bound(Input, n, 3); // retorna 2
    bsearch_lower_bound(Input, n, 8); // retorna 4
    bsearch_lower_bound(Input, n, 9); // retorna 5

    return 0;
}