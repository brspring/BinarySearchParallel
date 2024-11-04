#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "geraInput.h"

#define THREAD_COUNT 1
#define INPUT_SIZE 10
#define MAX_SIZE 10000000
#define NUM_MEDIÇÕES 1

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    long long *array;
    long long int n;
    long long value;
    long long int result;
    long long start;
    long long end;
} ParametroBuscaA;

void* worker_thread_A(void *arg) {
    ParametroBuscaA *param = (ParametroBuscaA*)arg;
    long long int low = param->start;
    long long int high = param->end;
    long long *arr = param->array;
    long long X = param->value;
    
    while (low < high) {
        long long mid = low + (high - low) / 2;
        if (X <= arr[mid]) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    pthread_mutex_lock(&queue_mutex);
    // Armazena o menor índice encontrado pela thread em `param->result`
    if((low <= param->n && arr[low] >= X))
        param->result = low;
    else{    
        printf("mudou\n");
        param->result = param->n;
    }
    pthread_mutex_unlock(&queue_mutex);

    return NULL;
}

int bsearch_lower_bound_A(long long array[], int n, long long value) {
    pthread_t threads[THREAD_COUNT];
    ParametroBuscaA params[THREAD_COUNT];
    long long int segment_size = n / THREAD_COUNT;

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

// int main() {
//     long long *Input = gerarVetor(INPUT_SIZE, MAX_SIZE, 1);
//     long long int n = INPUT_SIZE;
//     long long value = 255555;

//     long long int resultado = bsearch_lower_bound_A(Input, n, value);
//     printf("resultado = %lld\n", resultado);

//     return 0;
// }

void preencher_vetor_temporario(long long *temp_array, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        temp_array[i] = rand() % MAX_SIZE;
    }
}

int main() {
    srand(time(NULL));
    long long *InputOriginal = gerarVetor(INPUT_SIZE, MAX_SIZE, 1);
    long long int n = INPUT_SIZE;
    long long value = 100000000;

    long long *temp_array = (long long*)malloc(INPUT_SIZE * sizeof(long long));
    preencher_vetor_temporario(temp_array, INPUT_SIZE);

    double tempo_total = 0.0;  // Para armazenar o tempo total

    for (int i = 0; i < NUM_MEDIÇÕES; i++) {
        long long *Input = (long long*)malloc(INPUT_SIZE * sizeof(long long));
        memcpy(Input, InputOriginal, INPUT_SIZE * sizeof(long long));
        
        preencher_vetor_temporario(temp_array, INPUT_SIZE);

        // Início da medição de tempo
        clock_t inicio = clock();

        long long int resultado = bsearch_lower_bound_A(Input, n, value);

        // Fim da medição de tempo
        clock_t fim = clock();

        double tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;
        tempo_total += tempo_gasto;
        printf("valor = %d - resultado = %lld, Tempo = %.6f segundos\n", value, resultado, tempo_gasto);

        free(Input);
    }

    printf("Tempo médio = %.6f segundos\n", tempo_total / NUM_MEDIÇÕES);

    free(InputOriginal);
    free(temp_array);
    return 0;
}