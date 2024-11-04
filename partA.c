#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "geraInput.h"

#define THREAD_COUNT 4
#define QUEUE_SIZE 100000
#define INPUT_SIZE 16000000
#define MAX_SIZE 1000000

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    long long *array;
    int n;
    long long value;
    int result;
    long long start;
    long long end;
} ParametroBuscaA;

void* worker_thread_A(void *arg) {
    ParametroBuscaA *param = (ParametroBuscaA*)arg;
    int low = param->start;
    int high = param->end;
    long long *arr = param->array;
    long long X = param->value;
    
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (X <= arr[mid]) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    pthread_mutex_lock(&queue_mutex);
    // Armazena o menor índice encontrado pela thread em `param->result`
    if((low < param->n && arr[low] >= X))
        param->result = low;
    else{    
        param->result = -1;
    }
    pthread_mutex_unlock(&queue_mutex);

    return NULL;
}

int bsearch_lower_bound_A(long long array[], int n, long long value) {
    pthread_t threads[THREAD_COUNT];
    ParametroBuscaA params[THREAD_COUNT];
    int segment_size = n / THREAD_COUNT;

    // Cria as threads e divide o array em segmentos
    for (int i = 0; i < THREAD_COUNT; i++) {
        params[i].array = array;
        params[i].n = n;
        params[i].value = value;
        params[i].result = -1;
        params[i].start = i * segment_size;
        params[i].end = (i == THREAD_COUNT - 1) ? n : (i + 1) * segment_size;
        
        pthread_create(&threads[i], NULL, worker_thread_A, &params[i]);
    }

    // Espera as threads terminarem
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // Busca o menor índice válido encontrado entre todas as threads
    int final_result = -1;
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (params[i].result != -1 && (final_result == -1 || params[i].result < final_result)) {
            final_result = params[i].result;
        }
    }

    return final_result;
}

int main() {
    long long Input[] = { 0, 2, 5, 7, 8, 9, 9, 9, 9, 10, 19, 20 };
    int n = 12;
    long long value = 21;

    int resultado = bsearch_lower_bound_A(Input, n, value);
    printf("resultado = %d\n", resultado);

    return 0;
}