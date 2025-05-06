#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "../ring.h"  // Your rbuf struct + rbuf_write + rbuf_read

#define BUF_SIZE 10*32
#define NUM_MESSAGES 10
#define MAX_MSG_SIZE 32

rbuf* ring;

void* producer(void* arg) {
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        char msg[MAX_MSG_SIZE];
        snprintf(msg, MAX_MSG_SIZE, "msg-%d", i);

        // Spin until write succeeds
        while (rbuf_write(ring, msg, strlen(msg) + 1) != SUCCESS) {
            sched_yield();
        }
        printf("[Producer] Wrote: %s\n", msg);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        char msg[MAX_MSG_SIZE];

        // Spin until read succeeds
        while (rbuf_read(ring, MAX_MSG_SIZE, msg) != SUCCESS) {
            sched_yield();
        }
        printf("           [Consumer] Read: %s\n", msg);
    }
    return NULL;
}

int main() {
    ring = rbuf_init(BUF_SIZE);
    if (!ring) {
        fprintf(stderr, "Failed to initialize buffer\n");
        return 1;
    }

    pthread_t producer_thread, consumer_thread;

    pthread_create(&consumer_thread, NULL, consumer, NULL);
    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    free((void*)ring->buf);
    free(ring);

    return 0;
}

