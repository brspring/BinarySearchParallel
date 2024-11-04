#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "geraInput.h"

#define THREAD_COUNT 8
#define QUEUE_SIZE 100000
#define INPUT_SIZE 16000000
#define MAX_SIZE 1000000
#define NUM_COPIAS 10

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t completion_cond = PTHREAD_COND_INITIALIZER;

// Estrutura para armazenar parâmetros da busca
typedef struct {
    long long *array;
    long long int n;
    long long value;
    long long int *Pos;
    long long int index;
} ParametroBusca;

// Fila de tarefas e variáveis para controle
ParametroBusca *task_queue[QUEUE_SIZE];
long long int queue_start = 0, queue_end = 0, queue_size = 0;
long long int completed_tasks = 0;
long long int total_tasks = 0;

// Função que as threads do pool irão executar
void* worker_thread(void *arg) {
    long long int thread_id = *((long long int*)arg);
    //printf("ThreadID = %d FUNCIONANDO!\n", thread_id);
    while (1) {
        ParametroBusca *task;

        // Bloqueia o mutex para acessar a fila de tarefas
        pthread_mutex_lock(&queue_mutex);
        while (queue_size == 0) {
            // Aguarda até que haja uma tarefa disponível
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        // Remove uma tarefa da fila
        task = task_queue[queue_start];
        // printf("Thread %d - Tarefa %d\n", thread_id, queue_start);
        queue_start = (queue_start + 1) % QUEUE_SIZE;
        queue_size--;

        pthread_mutex_unlock(&queue_mutex);

        // Executa a busca binária na tarefa
        long long int mid;
        long long int low = 0;
        long long int high = task->n;
        long long *arrLocal = task->array;
        long long X = task->value;

        while (low < high) {
            mid = low + (high - low) / 2;
            if (X <= arrLocal[mid]) {
                high = mid;
            } else {
                low = mid + 1;
            }
        }
        if (low < task->n && arrLocal[low] < X)
            low++;
        
        task->Pos[task->index] = low;

        // Libera a memória da tarefa após a execução
        free(task);

        // Atualiza o contador de tarefas concluídas
        pthread_mutex_lock(&queue_mutex);
        completed_tasks++;
        if (completed_tasks == total_tasks) {
            pthread_cond_signal(&completion_cond);  // Sinaliza que todas as tarefas foram concluídas
        }
        pthread_mutex_unlock(&queue_mutex);
    }
}

void add_task_to_queue(ParametroBusca *task) {
    pthread_mutex_lock(&queue_mutex);

    // Adiciona a tarefa à fila de tarefas
    task_queue[queue_end] = task;
    queue_end = (queue_end + 1) % QUEUE_SIZE;
    queue_size++;

    pthread_cond_signal(&queue_cond);  // Sinaliza para as threads que há uma nova tarefa
    pthread_mutex_unlock(&queue_mutex);
}

// Função principal que adiciona as tarefas ao pool de threads
long long int* bsearch_lower_bound_B(long long array[], int n, long long Q[], long long  nQ, long long int *Pos) {
    total_tasks = nQ;  // Define o número total de tarefas que devem ser concluídas

    for (int i = 0; i < nQ; i++) {
        // Cria um novo parâmetro de busca para cada elemento de Q
        ParametroBusca *param = malloc(sizeof(ParametroBusca));
        param->array = array;
        param->value = Q[i];
        param->Pos = Pos;
        param->n = n;
        param->index = i;

        // Adiciona a tarefa ao pool de threads
        add_task_to_queue(param);
    }

    // Aguarda todas as tarefas serem concluídas
    pthread_mutex_lock(&queue_mutex);
    while (completed_tasks < total_tasks) {
        pthread_cond_wait(&completion_cond, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);

    return Pos;
}

void printPos(int *Pos, int nQ) {
    for(int i=0; i<nQ; i++) {
        printf("Pos[%d] = %d\n", i, Pos[i]);
    }
}


int main() {
    long long int *Pos = (long long int *)malloc(QUEUE_SIZE * sizeof(long long int));

    long long *InputG = (long long *)malloc(INPUT_SIZE * NUM_COPIAS * sizeof(long long));
    long long *QG = (long long *)malloc(QUEUE_SIZE * NUM_COPIAS * sizeof(long long));

    long long *Input = gerarVetor(INPUT_SIZE, MAX_SIZE, 1);
    long long *Q = gerarVetor(QUEUE_SIZE, MAX_SIZE, 0);

    for (int i = 0; i < NUM_COPIAS; i++) {
        memcpy(&InputG[i * INPUT_SIZE], Input, INPUT_SIZE * sizeof(long long));
        memcpy(&QG[i * QUEUE_SIZE], Q, QUEUE_SIZE * sizeof(long long));
    }

    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        int *idx = malloc(sizeof(int));
        *idx = i;
        pthread_create(&threads[i], NULL, worker_thread, idx);
    }

    double total_time = 0.0;
    for (int i = 0; i < NUM_COPIAS; i++) {
        clock_t start = clock();
        bsearch_lower_bound_B(&InputG[i * INPUT_SIZE], INPUT_SIZE, &QG[i * QUEUE_SIZE], QUEUE_SIZE, Pos);
        clock_t end = clock();

        double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
        total_time += time_taken;
        printf("Execução %d: %f segundos\n", i + 1, time_taken);
    }

    printf("Tempo médio: %f segundos\n", total_time / NUM_COPIAS);

    return 0;
}