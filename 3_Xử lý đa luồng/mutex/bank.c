#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int credit = 300;
pthread_mutex_t credit_mutex;

void* withdraw(void* arg) {
    pthread_mutex_lock(&credit_mutex);
    printf("Before withdrawing, I had %d$\n", credit); 
    int value = credit;
    printf("Withdrawing...\n");
    sleep(1);
    value -= 50;
    printf("Now I have %d$\n", value);
    credit = value; 
    pthread_mutex_unlock(&credit_mutex);
}

void* transfer(void* arg) {
    pthread_mutex_lock(&credit_mutex);
    printf("Before transfering, he had %d$\n", credit);
    int value = credit;
    printf("Transfering...\n");
    sleep(1);
    value += 100;
    printf("Now he has %d$\n", value); 
    credit = value;
    pthread_mutex_unlock(&credit_mutex);
}


int main() {
    pthread_mutex_init(&credit_mutex, NULL);

    pthread_t tid[2];
    pthread_create(&tid[0], NULL, transfer, NULL);
    pthread_create(&tid[1], NULL, withdraw, NULL);

    pthread_join(tid[1], NULL);
    printf("---------------------------\nFinally, your credit is %d$\n", credit);
    return 0;
}