#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct queue{
    void*(*run)(void*);
    void* argv;
}queue_t;

typedef struct queue_pool{
    int head;
    int tail;
    int size;
    int capacity;
    int waiting_threads;
    int shutdown;           //0 不退出; 1 退出
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    queue_t* tasks;
}queue_pool_t;

queue_pool_t* queue_create(int qsize, int tsize);
bool pool_add(queue_pool_t* q, queue_t* task);
bool queue_destory(queue_pool_t *q);

#ifdef __cplusplus
}
#endif

#endif