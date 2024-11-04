#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "geraInput.h"

#define THREAD_COUNT 2
#define QUEUE_SIZE 100000
#define INPUT_SIZE 16000000
#define MAX_SIZE 1000000
#define NUM_COPIES 10

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t completion_cond = PTHREAD_COND_INITIALIZER;

// Estrutura para armazenar parâmetros da busca
typedef struct {
    long long *array;
    int n;
    long long value;
    int *Pos;
    int index;
} ParametroBusca;

// Fila de tarefas e variáveis para controle
ParametroBusca *task_queue[QUEUE_SIZE];
int queue_start = 0, queue_end = 0, queue_size = 0;
int completed_tasks = 0;
int total_tasks = 0;

// Função que as threads do pool irão executar
void* worker_thread(void *arg) {
    int thread_id = *((int*)arg);
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
        int mid;
        int low = 0;
        int high = task->n;
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
int* bsearch_lower_bound_B(long long array[], int n, long long Q[], int nQ, int *Pos) {
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

double medirTempoExecucao(long long *input, long long *q, int *pos, int input_size, int queue_size) {
    printf("executando...\n");
    clock_t inicio = clock();
    bsearch_lower_bound_B(input, input_size, q, queue_size, pos);
    clock_t fim = clock();
    return (double)(fim - inicio) / CLOCKS_PER_SEC;
}

int main() {
    int *Pos = (int *)malloc(NUM_COPIES * QUEUE_SIZE * sizeof(int));

    //gera as cópias de Input e Q em InputG e QG
    long long *InputG = (long long *)malloc(NUM_COPIES * INPUT_SIZE * sizeof(long long));
    long long *QG = (long long *)malloc(NUM_COPIES * QUEUE_SIZE * sizeof(long long));
    
    long long *input_original = gerarVetor(INPUT_SIZE, MAX_SIZE, 1);
    long long *q_original = gerarVetor(QUEUE_SIZE, MAX_SIZE, 0);

    //preenche InputG e QG com as cópias de input_original e q_original
    for (int i = 0; i < NUM_COPIES; i++) {
        memcpy(InputG + i * INPUT_SIZE, input_original, INPUT_SIZE * sizeof(long long));
        memcpy(QG + i * QUEUE_SIZE, q_original, QUEUE_SIZE * sizeof(long long));
    }

    //inicializa o pool de threads
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        int *idx = malloc(sizeof(int));
        *idx = i;
        pthread_create(&threads[i], NULL, worker_thread, idx);
    }

    double soma_tempos = 0.0;
    for (int i = 0; i < NUM_COPIES; i++) {
        double tempo_execucao = medirTempoExecucao(
            InputG + i * INPUT_SIZE,
            QG + i * QUEUE_SIZE,
            Pos + i * QUEUE_SIZE,
            INPUT_SIZE,
            QUEUE_SIZE
        );
        soma_tempos += tempo_execucao;
    }

    //calcula a média dos tempos de execução
    double media_tempo = soma_tempos / NUM_COPIES;
    printf("Tempo médio de execução: %.6f segundos\n", media_tempo);

    //libera memória alocada
    free(Pos);
    free(InputG);
    free(QG);
    free(input_original);
    free(q_original);

    return 0;
}