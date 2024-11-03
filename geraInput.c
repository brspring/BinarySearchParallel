#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "geraInput.h"

// Função de comparação para `qsort` (ordena em ordem crescente)
int compare(const void *a, const void *b) {
    long long val1 = *(const long long*)a;
    long long val2 = *(const long long*)b;
    return (val1 > val2) - (val1 < val2);
}

// Gera um vetor com size elementos
long long* gerarVetor(int size, long long max_value, int ordena) {
    long long* vetor = (long long*)malloc(size * sizeof(long long));
    if (vetor == NULL) {
        perror("Erro ao alocar memória para o vetor Q");
        exit(EXIT_FAILURE);
    }
    
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        vetor[i] = rand() % max_value; // Valores aleatórios até max_value
    }
    
    // Ordena o vetor usando qsort
    if(ordena == 1)
        qsort(vetor, size, sizeof(long long), compare);

    return vetor;
}


// Gera o vetor Pos com size elementos inicializados com -1
int* gerarPos(int size) {
    int* vector = (int*)malloc(size * sizeof(int));
    if (vector == NULL) {
        perror("Erro ao alocar memória para o vetor Pos");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < size; i++) {
        vector[i] = -1; // Inicializa com -1 para indicar posições não definidas
    }
    
    return vector;
}

void printVetor(int size, long long *vetor) {
    for(int i=0; i<size; i++) {
        printf("Vetor[%d] = %d\n", i, vetor[i]);
    }
}