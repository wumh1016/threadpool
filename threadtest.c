#include "threadpool.h"

void* fun(void* data);

int main(int argc, char const *argv[])
{
    threadpool_t* pool = threadpool_create(4, 120);
    int arr[100] = {0};
    for(int i=0; i<100; i++){
        arr[i] = i;
    }

    for(int i=0; i<10; i++){
        task_t task;
        task.data = (void*)&arr[i];
        task.func = fun;
        tasks_add(pool, task);
    }

    threadpool_destory(pool);
    return 0;
}

void* fun(void* data)
{
    int num = *((int*)data) + 1;
    printf("hello world : %d\n", num);
    return NULL;
}
