#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define bool int
#define true 1
#define false 0

typedef struct task{
    void*(*func)(void*);
    void*data;
}task_t;

typedef struct worker{
    pthread_t threadid;
}worker_t;

typedef struct threadpool{
    int head;
    int tail;
    int size;
    int capacity;
    int worksize;
    int waitings;
    int quit;           // 0:不退出; 1:退出
    worker_t* workers;
    task_t* tasks;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}threadpool_t;

threadpool_t* threadpool_create(int work_size, int task_size);
bool tasks_add(threadpool_t* pool, task_t task);
bool threadpool_destory(threadpool_t* pool);
bool task_push_tail(threadpool_t* pool, task_t task);
task_t* task_pop_head(threadpool_t* pool);
bool tasks_is_full(threadpool_t* pool);
bool tasks_is_empty(threadpool_t* pool);

#endif
