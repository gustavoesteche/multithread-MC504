#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BABOONS 20 // Quantidade total de babínos
#define MAX_BABOONS_ON_ROPE 5 // Capacidade máxima de babuínos na corda

#define ESQ 1
#define ESQ_DIR 2
#define DIR_ESQ 3
#define DIR 4
#define ESQ_OK 5
#define DIR_OK 6

sem_t mutex; // Região de exclusão
sem_t leftQueue, rightQueue; // Filas de prioridade para babuínos

int count = 0;          // Número de babuínos na corda
int direction = 0;      // 0: nenhum, -1: esquerda->direita, 1: direita->esquerda
int leftWaiting = 0;    // Número de babuínos esperando para cruzar da esquerda para a direita
int rightWaiting = 0;   // Número de babuínos esperando para cruzar da direita para a esquerda

char rope[MAX_BABOONS_ON_ROPE]; // Representação da corda (visualizador)
int baboon_state[MAX_BABOONS];  // Armazena o estado de cada babuíno individualmente (visualizador)

// Função para imprimir o estado individual de cada babuíno em um instante
void print_each_baboon() {
    for(int i = 0; i < MAX_BABOONS; i++) {
        switch (baboon_state[i]) {
            case ESQ:
                printf("\U0001f412 %d | ESQUERDA\n", i+1); break;
            case ESQ_DIR:
                printf("\U0001f412 %d | ESQUERDA -> DIREITA\n", i+1); break;
            case DIR_ESQ:
                printf("\U0001f412 %d | DIREITA -> ESQUERDA\n", i+1); break;
            case DIR:
                printf("\U0001f412 %d | DIREITA\n", i+1); break;
            case ESQ_OK:
                printf("\U0001f412 %d | ESQUERDA \U0001f197\n", i+1); break;
            case DIR_OK:
                printf("\U0001f412 %d | DIREITA \U0001f197\n", i+1); break;
            default:
                printf("\U0001f412 %d | zzz...\n", i+1); break;

        }
    }
}

// Função para imprimir o estado atual da corda e das filas de espera
void print_state() {
    printf("\nEstado atual:\n");
    printf("Corda: [");
    for (int i = 0; i < MAX_BABOONS_ON_ROPE; i++) {
        if (rope[i] == 'L' || rope[i] == 'R') {
            printf("%c", rope[i]);
        } else {
            printf(" ");
        }
    }
    printf("]\n");
    print_each_baboon();
    printf("Esperando à esquerda: %d\n", leftWaiting);
    printf("Esperando à direita: %d\n", rightWaiting);
}

void* cross_left_to_right(void* arg) {
    sem_wait(&mutex);
    int* state = (int*)arg;
    *state = ESQ;
    leftWaiting++;
    while (direction == 1 || count == MAX_BABOONS_ON_ROPE) {
        sem_post(&mutex);
        sem_wait(&leftQueue);
        sem_wait(&mutex);
    }
    *state = ESQ_DIR;
    leftWaiting--;
    direction = -1;
    rope[count] = 'R'; // Adiciona babuíno à corda
    count++;
    sleep(1);
    print_state();
    printf("Babuíno indo para a direita!\n"); 
    sem_post(&mutex);

    sleep(1);

    sem_wait(&mutex);
    *state = DIR_OK;
    count--;
    rope[count] = ' '; // Remove babuíno da corda
    if (count != 0 && rightWaiting == 0){
        for (int i = 0; i < leftWaiting && i < MAX_BABOONS_ON_ROPE - count; i++) {
            sem_post(&leftQueue);
        }
    }else if (count == 0) {
        direction = 0;
        if (rightWaiting > 0) {
            for (int i = 0; i < rightWaiting && i < MAX_BABOONS_ON_ROPE; i++) {
                sem_post(&rightQueue);
            }
        } else if (leftWaiting > 0) {
            for (int i = 0; i < leftWaiting && i < MAX_BABOONS_ON_ROPE; i++) {
                sem_post(&leftQueue);
            }
        }
    }
    sleep(1);
    print_state();
    sem_post(&mutex);

    return NULL;
}

void* cross_right_to_left(void* arg) {
    sem_wait(&mutex);
    int* state = (int*)arg;
    *state = DIR;
    rightWaiting++;
    while (direction == -1 || count == MAX_BABOONS_ON_ROPE) {
        sem_post(&mutex);
        sem_wait(&rightQueue);
        sem_wait(&mutex);
    }
    *state = DIR_ESQ;
    rightWaiting--;
    direction = 1;
    rope[count] = 'L'; // Adiciona babuíno à corda
    count++;
    sleep(1);
    print_state();
    printf("Babuíno indo para a esquerda!\n"); 
    sem_post(&mutex);
    
    sleep(1);

    sem_wait(&mutex);
    *state = ESQ_OK;
    count--;
    rope[count] = ' '; // Remove babuíno da corda

    if (count != 0 && leftWaiting == 0){
        for (int i = 0; i < rightWaiting && i < MAX_BABOONS_ON_ROPE - count; i++) {
            sem_post(&rightQueue);
        }
    }else if (count == 0) {
        direction = 0;
        if (leftWaiting > 0) {
            for (int i = 0; i < leftWaiting && i < MAX_BABOONS_ON_ROPE; i++) {
                sem_post(&leftQueue);
            }
        } else if (rightWaiting > 0) {
            for (int i = 0; i < rightWaiting && i < MAX_BABOONS_ON_ROPE; i++) {
                sem_post(&rightQueue);
            }
        }
    }
    sleep(1);
    print_state();
    sem_post(&mutex);

    return NULL;
}

int main() {
    sem_init(&mutex, 0, 1);
    sem_init(&leftQueue, 0, 0);
    sem_init(&rightQueue, 0, 0);

    // Inicializa a corda como vazia
    for (int i = 0; i < MAX_BABOONS_ON_ROPE; i++) {
        rope[i] = ' ';
    }

    // Inicializa os babuinos dormindo
    for (int i = 0; i < MAX_BABOONS; i++) {
        baboon_state[i] = 0;
    }

    int n_baboons = MAX_BABOONS; // Número de babuínos
    pthread_t baboons[MAX_BABOONS];
     

    // Simula babuínos cruzando a corda em ordem aleatória
    for (int i = 0; i < MAX_BABOONS; i++) {
        if (rand() % 2 == 0)
            pthread_create(&baboons[i], NULL, cross_left_to_right, (void*)&baboon_state[i]);
        else
            pthread_create(&baboons[i], NULL, cross_right_to_left, (void*)&baboon_state[i]);
    }

    for (int i = 0; i < MAX_BABOONS; i++) {
        pthread_join(baboons[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&leftQueue);
    sem_destroy(&rightQueue);

    return 0;
}

