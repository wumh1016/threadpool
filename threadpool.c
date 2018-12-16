#include "threadpool.h"

#define QUEUE_SIZE 65536

static void* threadpool_thread(void* threadpool);

static bool pool_push(queue_pool_t* q, queue_t* task);
static queue_t* pool_pop(queue_pool_t* q);
static bool queue_is_full(queue_pool_t* q);
static bool queue_is_empty(queue_pool_t* q);

queue_pool_t* queue_create(int qsize, int tsize)
{
    queue_pool_t* q = (queue_pool_t*)malloc(sizeof(queue_pool_t);
    if(NULL == q){
        printf("malloc error!\n");
        return NULL;
    }
    if(size < 0) qsize = QUEUE_SIZE;
    q->tasks = (queue_t*)malloc(QUEUE_SIZE * sizeof(queue_t));
    q->capacity = QUEUE_SIZE;
    q->head = q->tail = q->size = q->shutdown = q->waiting_threads = 0;
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->cond), NULL);

    q->threads = (pthread_t *)malloc(tsize * sizeof(pthread_t));
    for(int i=0; i<tsize; i++){
        if(pthread_create(&(q->threads[i]), NULL, threadpool_thread, (void*)q) != 0){
            printf("pthread_create error!\n");
            queue_destory(q);
            return NULL;
        }
    }
    return q;
}

bool pool_add(queue_pool_t* q, queue_t* task)
{
    if(NULL == q || NULL == task->run)  return false;
    if(!pool_push(q, task)) return false;
    return true;
}

bool pool_push(queue_pool_t* q, queue_t* task)
{
    if(queue_is_full(q)) return false;
    pthread_mutex_lock(&(q->mutex));
    q->tasks[q->tail] = *task;
    q->tail = (q->tail+1) % (q->capacity);
    q->size ++;
    if(q->waiting_threads > 0){
        pthread_cond_signal(&(q->cond));
    }
    pthead_mutex_unlock(&(q->mutex));

    return true;
}

queue_t* pool_pop(queue_pool_t* q)
{
    if(queue_is_empty(q)) return NULL;
    queue_t* task = NULL;
    pthread_mutex_lock(&(q->mutex));
    q->waiting_threads ++;
    pthread_cond_wait(&(q->cond), &(q->mutex))
    task->run = q->tasks[q->head].run;
    task->argv = q->tasks[q->head].argv;
    q->waiting_threads --;
    pthread_mutex_unlock(&(q->mutex));
    return task;
}

void* threadpool_thread(void* threadpool)
{
    queue_t* task = NULL;
    queue_pool_t *q = (queue_pool_t*)threadpool;
    while(1){
        task = pool_pop(q);
        if(NULL != task){
            task->run(task->argv);
        }
        if(q->shutdown == 1){
            printf("thread %d is exit.\n", pthread_self());
            break;
        }
    }
    return NULL;
}

bool queue_is_full(queue_pool_t* q)
{
    return q->size == q->capacity;
}

bool queue_is_empty(queue_pool_t* q)
{
    return q->size == 0;
}

bool queue_destory(queue_pool_t *q)
{
    q->shutdown = 1;
    free(q->tasks);
    free(q->threads);
    free(q);
    return true;
}