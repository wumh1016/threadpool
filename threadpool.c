#include "threadpool.h"

#define MAX_WORKS 8
#define MAX_TASKS 65536

static void* handler(void*arg);

threadpool_t* threadpool_create(int work_size, int task_size)
{
    threadpool_t* pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if(NULL == pool){
        perror("threadpool malloc!");
        return NULL;
    }

    if(task_size <= 0 || task_size > MAX_TASKS) task_size = MAX_TASKS;
    pool->tasks = (task_t*)malloc(task_size * sizeof(task_t));
    if(NULL == pool->tasks){
        perror("tasks malloc!");
        threadpool_destory(pool);
        return NULL;
    }

    if(work_size <=0 || work_size > MAX_WORKS) work_size = MAX_WORKS;
    pool->workers = (worker_t*)malloc(work_size * sizeof(worker_t));
    if(NULL == pool->workers){
        perror("workers malloc!");
        threadpool_destory(pool);
        return NULL;
    }

    for(int i=0; i<work_size; i++){
        if(0 != pthread_create(&(pool->workers[i].threadid), NULL, handler, (void*)pool)){
            perror("pthread_create!");
            threadpool_destory(pool);
            return NULL;
        }
    }

    pool->size = pool->head = pool->tail = pool->waitings = 0;
    pool->capacity = task_size;
    pool->worksize = work_size;
    pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->cond), NULL);

    return pool;
}

bool tasks_add(threadpool_t* pool, task_t task)
{
    if(NULL == pool || NULL == task.func){
        printf("add task error.\n");
        return false;
    }

    do{
        if(pool->quit){
            printf("pool->quit");
            return false;
        }
        if(!task_push_tail(pool, task)){
            printf("push_tail in add error\n");
            return false;
        }
    }while(0);

    return true;
}

bool threadpool_destory(threadpool_t* pool)
{
    pool->quit = 1;
    for(int i=0; i<pool->worksize; i++){
        printf("will destory thread\n");
        if(0 != pthread_join(pool->workers[i].threadid, NULL)){
            perror("pthread_join");
        }
    }

    if(pool->workers)   free(pool->workers);
    if(pool->tasks)     free(pool->tasks);
    if(pool)            free(pool);

    return true;
}

bool task_push_tail(threadpool_t* pool, task_t task)
{
    if(tasks_is_full(pool))     return false;

    pthread_mutex_lock(&(pool->mutex));

    pool->tasks[pool->tail].data = task.data;
    pool->tasks[pool->tail].func = task.func;
    pool->size ++;

    if(pool->waitings > 0) pthread_cond_signal(&(pool->cond));

    pool->tail = (pool->tail + 1) % pool->capacity;
    pthread_mutex_unlock(&(pool->mutex));
    return true;
}

task_t* task_pop_head(threadpool_t* pool)
{
    task_t* task = NULL;
    pthread_mutex_lock(&(pool->mutex));

    if(tasks_is_empty(pool)){
        pool->waitings ++;
        pthread_cond_wait(&(pool->cond), &(pool->mutex));
        pool->waitings --;
    }

    task = &(pool->tasks[pool->head]);
/*     task->func = pool->tasks[pool->head].func;
    task->data = pool->tasks[pool->head].data; */
    pool->size --;
    pool->head = (pool->head + 1) % pool->capacity;

    pthread_mutex_unlock(&(pool->mutex));
    return task;
}

bool tasks_is_full(threadpool_t* pool)
{
    return pool->size == pool->capacity;
}

bool tasks_is_empty(threadpool_t* pool)
{
    return pool->size == 0;
}

void* handler(void*arg)
{
    threadpool_t* pool = (threadpool_t*)arg;

    while(1){
        task_t* task = task_pop_head(pool);
        if(NULL != task){
            task->func(task->data);
        }
        printf("%d thread size %d \n", pool->size, pthread_self());
        if(pool->quit && tasks_is_empty(pool)){
            printf("exit thread\n");
            break;
        }
    }

    return NULL;
}
