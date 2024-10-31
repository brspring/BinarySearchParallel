#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// ja add a flag -lpthread no Makefile
#include <pthread.h>
#define THREAD_COUNT 4

pthread_barrier_t mybarrier;


int bsearch_lower_bound_A(long long arr[], int N, long long X)
{
    int mid;
    int low = 0;
    int high = N;
 
    while (low < high) {
        mid = low + (high - low) / 2;

        if (X <= arr[mid]) {
            high = mid;
        }
        else {
            low = mid + 1;
        }
    }

    if(low < N && arr[low] < X)
       low++;

    return low;
}

int* bsearch_lower_bound_B(long long array[], int n, long long Q[], int nQ, int Pos[])
{
    for (int i = 0; i < nQ; i++){
        // criar uma thread para cada elemento do vetor Q

        Pos[i] = bsearch_lower_bound_A(array, n, Q[i]);
        // retorna a posicao do primeiro elemento do vetor Q
    }
    pthread_barrier_wait(&mybarrier);

    return Pos;
}

int main() {
    long long Input[] = { 0, 2, 5, 7, 8 };
    int n = 5;

    long long Q[] = { 3, 8, 9 };
    int nQ = 3;
    pthread_t ids[nQ];
    int short_ids[nQ];

    int *Pos = (int *)malloc(nQ * sizeof(int)); // Aloca memória para o array Pos

    // Pos = bsearch_lower_bound_B(Input, n, Q, nQ, Pos);

    // for (int i = 0; i < nQ; i++) {
    //     printf("Pos[%d] = %d\n", i, Pos[i]);
    // }

    // nQ = 2;  
    // Pos = bsearch_lower_bound_B(Input, n, Q, nQ, Pos);
    //   // retorna em Pos os valores {2, 4}

    // //printa os valores na segunda vez
    // for (int i = 0; i < nQ; i++) {
    //     printf("Pos2[%d] = %d\n", i, Pos[i]);
    // }

    srand(time(NULL));
    pthread_barrier_init(&mybarrier, NULL, nQ + 1);

    for (int i=0; i < nQ; i++) {
        short_ids[i] = i;
        pthread_create(&ids[i], NULL, bsearch_lower_bound_A, &short_ids[i]);
    }

    pthread_barrier_wait(&mybarrier);

    printf("main() is going!\n");

    for (int i=0; i < THREAD_COUNT; i++) {
        pthread_join(ids[i], NULL);
    }

    pthread_barrier_destroy(&mybarrier);

    free(Pos); // Libera a memória alocada para Pos

    return 0;
}