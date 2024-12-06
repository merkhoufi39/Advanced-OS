#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Buffer;

void init_buffer(Buffer *buf) {
    buf->in = 0;
    buf->out = 0;
    buf->count = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->not_empty, NULL);
    pthread_cond_init(&buf->not_full, NULL);
}

void destroy_buffer(Buffer *buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->not_empty);
    pthread_cond_destroy(&buf->not_full);
}

void *producer(void *param) {
    Buffer *buf = (Buffer *)param;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&buf->mutex);

        while (buf->count == BUFFER_SIZE) {
            pthread_cond_wait(&buf->not_full, &buf->mutex);
        }

        buf->buffer[buf->in] = i;
        printf("Produced: %d\n", i);
        buf->in = (buf->in + 1) % BUFFER_SIZE;
        buf->count++;

        pthread_cond_signal(&buf->not_empty);
        pthread_mutex_unlock(&buf->mutex);

        sleep(1);
    }
    pthread_exit(0);
}

void *consumer(void *param) {
    Buffer *buf = (Buffer *)param;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&buf->mutex);

        while (buf->count == 0) {
            pthread_cond_wait(&buf->not_empty, &buf->mutex);
        }

        int item = buf->buffer[buf->out];
        printf("Consumed: %d\n", item);
        buf->out = (buf->out + 1) % BUFFER_SIZE;
        buf->count--;

        pthread_cond_signal(&buf->not_full);
        pthread_mutex_unlock(&buf->mutex);

        sleep(1);
    }
    pthread_exit(0);
}

int main() {
    Buffer buf;
    init_buffer(&buf);

    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, &buf);
    pthread_create(&consumer_thread, NULL, consumer, &buf);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    destroy_buffer(&buf);

    return 0;
}
