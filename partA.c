#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// ja add a flag -lpthread no Makefile
#include <pthread.h>
#define THREAD_COUNT 4

pthread_barrier_t mybarrier;

// Struct usada para passar dados para as threads
typedef struct {
    long long *array;
    int n;
    long long value;
    int *Pos;
    int index;
} ParametroBusca;

void* bsearch_lower_bound_Parallel(void *arg){
    // Recebe parâmetros da thread
    ParametroBusca *dados = (ParametroBusca *)arg;
    long long *arrLocal = dados->array;
    int nLocal = dados -> n;
    long long X = dados->value;

    // Variáveis do binary search
    int mid;
    int low = 0;
    int high = nLocal;

    // Busca
    while (low < high) {
        mid = low + (high - low) / 2;

        if (X <= arrLocal[mid]) {
            high = mid;
        }
        else {
            low = mid + 1;
        }
    }
    if(low < nLocal && arrLocal[low] < X)
       low++;
    
    //armazena o resultado local no vetor
    // esse index é para se uma thread terminar antes nao pegar a posicao de outra no vetor
    dados->Pos[dados->index] = low;
    printf("Thread %d chegou na barreira! Posição %ld para o valor %lld\n", dados->index, dados->Pos[dados->index], dados->value);

    // Espera todas as threads terminarem para seguirem com o programa. Caso contrário a impressão dos dados sairá errada (uma thread pode não terminar a tempo, por exemplo)
    pthread_barrier_wait(&mybarrier);
}

int* bsearch_lower_bound_B(long long array[], int n, long long Q[], int nQ, int *Pos)
{   
    pthread_t threads[THREAD_COUNT];
    int *index = malloc(sizeof(int));
    ParametroBusca parametros[3];
    // Inicializa a barreira com o número de threads
    pthread_barrier_init(&mybarrier, NULL, nQ);
    for (int i = 0; i < nQ; i++){
        // criar uma thread para cada elemento do vetor Q
        parametros[i].array = array;
        parametros[i].value = Q[i];
        parametros[i].Pos = Pos;
        parametros[i].n = n;
        parametros[i].index = i;
        pthread_create(&threads[i], NULL, bsearch_lower_bound_Parallel, &parametros[i]);
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < nQ; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destrói a barreira após o uso
    pthread_barrier_destroy(&mybarrier);

    return Pos;
}

void printPos(int *Pos, int nQ) {
    for(int i=0; i<nQ; i++) {
        printf("Pos[%d] = %d\n", i, Pos[i]);
    }
}

int main() {
    long long Input[] = { 0, 2, 5, 7, 8 };
    int n = 5;
    long long Q[] = { 3, 8, 9 };
    srand(time(NULL));

    // ---------------- TESTES ----------------
    // Retorna {2,4,5}
    int nQ = 3;
    int *Pos = (int *)malloc(nQ * sizeof(int)); // Aloca memória para o array Pos
    bsearch_lower_bound_B(Input, n, Q, nQ, Pos);
    printPos(Pos, nQ);

    // Retorna {2,4}
    nQ = 2;
    int *Pos2 = (int *)malloc(nQ * sizeof(int)); // Aloca memória para o array Pos
    bsearch_lower_bound_B(Input, n, Q, nQ, Pos2);
    printPos(Pos2, nQ);
    // ----------------------------------------

    free(Pos); // Libera a memória alocada para Pos
    return 0;
}