#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<pthread.h>
#include<assert.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<errno.h>

#define DEFAULT_TIME 3        //每10秒检测一次
#define MIN_WAIT_TASK_NUM 3   //如果queue_size >MIN_WAIT_TASK_NUM,添加新线程
#define DEFAULT_THREAD_VARY 3 //每次创建和销毁线程的个数
#define true 1
#define false 0

typedef struct{
    void *(*function)(void *);  //回调函数
    void *arg;                  //回调函数参数
}threadpool_task_t;             //子线程任务结构体

/* 描述线程池相关信息*/
typedef struct threadpool_t
{
    pthread_mutex_t lock;                 //锁住本结构体
    pthread_mutex_t thread_counter;       //记录忙状态线程个数的锁
    pthread_cond_t queue_not_full;        //当任务队列满时，添加任务的线程阻塞，等待此条件变量
    pthread_cond_t queue_not_empty;       //任务队列不为空时，通知等待任务的线程

    pthread_t *threads;                   //存放线程池中每个线程的tid， 数组
    pthread_t adjust_tid;                 //存管理线程tid
    threadpool_task_t *task_queue;        //任务队列

    int min_thr_num;         //线程池最小线程数
    int max_thr_num;         //线程池最大线程数
    int live_thr_num;        //当前存活线程个数
    int wait_exit_thr_num;    //需要销毁的线程个数
    int busy_thr_num;         //忙状态线程个数

    int queue_front;         //task_queue队头下标
    int queue_rear;          //task_queue队尾下标
    int queue_size;          //task_queue实际任务数量
    int queue_max_size;      //task_queue可容纳任务数上限

    int shutdown;            //标志位，线程池使用状态，true或者false

}threadpool_t;

/*方便打印调试*/
typedef struct{
    threadpool_t *pool;
    int index;
}info_t;

typedef struct{
    void *task_index;
    int pthread_index;
}info_task;
/*判断线程是否存活*/
int is_thread_alive(pthread_t tid)
{
    int kill_rc = pthread_kill(tid,0);   //发0信号测试线程是否存活
    if(kill_rc == ESRCH)
        return false;
    return true;
}

/*销毁线程池*/
int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL)
        return -1;
    if(pool->task_queue)
        free(pool->task_queue);

    if(pool->threads)
    {
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool=NULL;

    return 0;
}
/* 统计所有线程数量*/
int threadpool_all_threadnum(threadpool_t *pool)
{
    int all_threadnum = -1;
    pthread_mutex_lock(&(pool->lock));
    all_threadnum = pool->live_thr_num;
    pthread_mutex_unlock(&(pool->lock));
    return all_threadnum;
}

/* 统计忙线程数量*/
int threadpool_busy_threadnum(threadpool_t *pool)
{
    int busy_threadnum=-1;
    pthread_mutex_lock(&(pool->thread_counter));
    busy_threadnum=pool->busy_thr_num;
    pthread_mutex_unlock(&(pool->thread_counter));
    return busy_threadnum;
}


/* 向线程池中，添加一个任务*/
int threadpool_add(threadpool_t *pool,void*(function(void *arg)),void *arg)
{
   int ret=pthread_mutex_lock(&(pool->lock));
   if(ret!=0)
   {
       perror("mutex error");
       exit(1);
   }

   /* 如果队列满，阻塞等待*/
   while((pool->queue_size == pool->queue_max_size)&&(!pool->shutdown))
   {
       printf("queue full-----wait\n");
       pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
   }

   if(pool->shutdown)
       pthread_mutex_unlock(&(pool->lock));

   /* 清空工作线程 调用的回调函数的 参数arg*/
   if(pool->task_queue[pool->queue_rear].arg!=NULL)
   {
       free(pool->task_queue[pool->queue_rear].arg);
       pool->task_queue[pool->queue_rear].arg = NULL;
   }

   /* 添加任务到任务队列中*/
   pool->task_queue[pool->queue_rear].function = function;
   pool->task_queue[pool->queue_rear].arg = arg;
   printf("add task finish---task_queue[%d]",pool->queue_rear);
   pool->queue_rear=(pool->queue_rear + 1) % pool->queue_max_size;   //队尾指针移动
   pool->queue_size++;
   printf("---rear=%d,size=%d\n",pool->queue_rear,pool->queue_size);

   /*添加完任务后，队列不为空，唤醒线程池中 等待处理任务的线程*/
   pthread_cond_signal(&(pool->queue_not_empty));
   pthread_mutex_unlock(&(pool->lock));
   return 0;

}
/* 线程池中各个工作线程*/
void *threadpool_func(void *info_func)
{
    info_t *info=(info_t *)info_func;
    threadpool_t *pool= info->pool;
    threadpool_task_t task;
    
    while(true)
    {
        pthread_mutex_lock(&(pool->lock));
        /* 刚创建出线程，等待任务队列里的任务，没有任务的话，阻塞等待有任务时再唤醒接收任务*/
        while((pool->queue_size == 0) && (!pool->shutdown))
        {
            printf("thread 0x%X is waiting----threads[%d]\n",(unsigned int)pthread_self(),info->index);
            pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));
            /*参数1：等待条件变量信号唤醒，阻塞等待
 *              *参数2：阻塞等待期间对lock解锁，等待完成后重新加锁，防止死锁
 *                           * */
            /*清除指定数目的空闲线程，如果要结束的线程个数大于0，结束线程 */
            if(pool->wait_exit_thr_num > 0)
            {
                pool->wait_exit_thr_num--;

                /* 如果线程池中线程个数大于最小值时可以结束当前线程*/
                if(pool->live_thr_num > pool->min_thr_num)
                {
                    printf("thread 0x%X is exiting------threads[%d]\n",(unsigned int)pthread_self(),info->index);
                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(NULL);
                }
            }
        }
        /*如果指定了true ，要关闭线程池里的每个线程，自行退出处理 */
        if(pool->shutdown)
        {
            pthread_mutex_unlock(&(pool->lock));
                    printf("thread 0x%X is exiting------threads[%d]\n",(unsigned int)pthread_self(),info->index);
            pthread_exit(NULL);
        }

        /* 从任务队列中获取任务 ， 是一个出队操作*/
        task.function =pool->task_queue[pool->queue_front].function;
        task.arg=pool->task_queue[pool->queue_front].arg;

        pool->queue_front=(pool->queue_front + 1) % pool->queue_max_size;  //出队，模拟环形队列
        pool->queue_size--;

        /*通知可以有新任务添加进来*/
        pthread_cond_broadcast(&(pool->queue_not_full));

        /*任务取出后，立即将线程池锁释放*/
        pthread_mutex_unlock(&(pool->lock));

        /*执行任务  */
        printf("thread 0x%X start working-----threads[%d]\n",(unsigned int)pthread_self(),info->index);
        pthread_mutex_lock(&(pool->thread_counter));                  //忙状态线程数+1
        pool->busy_thr_num++;
        printf("busy_thr_num=%d,pid=0x%X\n",pool->busy_thr_num,(unsigned int)pthread_self());

        pthread_mutex_unlock(&(pool->thread_counter));
        info_task *info_arg=(info_task *)malloc(sizeof(info_task));//打印信息
        info_arg->pthread_index=info->index;
        info_arg->task_index=task.arg;
        (*(task.function))((void *)info_arg);               //执行回调函数任务
        //task.function((void *)info_arg);
        
        /* 任务处理结束  */ 
        printf("thread 0x%x end working----threads[%d]\n",(unsigned int)pthread_self(),info->index);
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num--;
        pthread_mutex_unlock(&(pool->thread_counter));
                                
     }
        pthread_exit(NULL);
}


/* 管理者线程 */
void *adjust_func(void *threadpool)
{
    int i;
    threadpool_t *pool = (threadpool_t *)threadpool;
    while(!pool->shutdown)
    {

        pthread_mutex_lock(&(pool->lock));       
        int queue_size = pool->queue_size;       // 关注任务数     
        int live_thr_num = pool->live_thr_num;   // 存活任务数
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        int busy_thr_num = pool->busy_thr_num;   //忙着任务数
        pthread_mutex_unlock(&(pool->thread_counter));

        /* 创建新线程算法 ： 处于忙状态线程占80%，且存活的线程数少于最大线程个数时*/
        float adjust_num=(float)busy_thr_num/(float)live_thr_num;
        if(adjust_num > 0.8 && live_thr_num < pool->max_thr_num)
        {
            pthread_mutex_lock(&(pool->lock));
            int add=0;
            printf("*********************new 3 threads***************************\n");
            printf("busy_thr_num=%d---------live_thr_num=%d\n",busy_thr_num,live_thr_num);
            /* 一次增加3个线程*/
            for(i=0;i < pool->max_thr_num && add < DEFAULT_THREAD_VARY
                    &&pool->live_thr_num < pool->max_thr_num; i++)
            {
                if(pool->threads[i] == 0 || !(is_thread_alive(pool->threads[i])))
                {
                    info_t *info=(info_t *)malloc(sizeof(info_t));
                    info->pool=pool;
                    info->index=i;
                    pthread_create(&(pool->threads[i]),NULL,threadpool_func,(void *)info);
                    printf("start 0x%X.....threads[%d]\n",(unsigned int)pool->threads[i],i);
                    add++;
                    pool->live_thr_num++;
                }
            }
            printf("*********************new 3 threads end***************************\n");           
            pthread_mutex_unlock(&(pool->lock));
        }

        /*销毁多余的空闲线程 算法: 忙状态线程占存活线程50%不到 且 存活的线程数大于最小线程数*/
        if( (live_thr_num - busy_thr_num>busy_thr_num)&& (live_thr_num > pool->min_thr_num))
        {
            //一次性销毁3个线程
            printf("*********************destroy 3 threads ***************************\n");           
            printf("busy_thr_num=%d---------live_thr_num=%d\n",busy_thr_num,live_thr_num);
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY; //要销毁的线程数
            pthread_mutex_unlock(&(pool->lock));
 
            for(i=0;i < DEFAULT_THREAD_VARY;i++)
                  pthread_cond_signal(&(pool->queue_not_empty));  //唤醒线程，它们会自动结束 
 
            printf("*********************destroy 3 threads end***************************\n");           
        }
         //sleep(DEFAULT_TIME);                     // 定时对线程池管理
        usleep(20000);
   }
       return NULL;
}

/*线程销毁函数*/

int threadpool_destroy(threadpool_t *pool)
{
    int i;
    if(pool == NULL)
        return -1;
    pool->shutdown=true;
    printf("*****************    destroy  **************\n");
    /*先销毁管理线程*/
    printf("pthread_join ----adjust_pid\n");
    pthread_join(pool->adjust_tid,NULL);

    for(i=0 ; i < pool->live_thr_num;i++)
        pthread_cond_broadcast(&(pool->queue_not_empty));     //通知所有空闲线程
    
    for(i=0;i<pool->live_thr_num;i++)
        pthread_join(pool->threads[i],NULL);
    
    printf("pthread_join ----pthread_pool\n");
    threadpool_free(pool);

    return 0;
}

/* 创建线程池*/
threadpool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size)
{
    int i;
    threadpool_t *pool=NULL;
    do
    {
        if((pool = (threadpool_t *)malloc(sizeof(threadpool_t)))==NULL)
        {
            printf("malloc threadpool fail! ");
            break;
        }
        pool->min_thr_num=min_thr_num;
        pool->max_thr_num=max_thr_num;
        pool->busy_thr_num=0;
        pool->live_thr_num=min_thr_num;    //初始存活线程为10个
        pool->queue_size=0;                //现在有0个任务
        pool->queue_front=0;
        pool->queue_rear=0;
        pool->shutdown =false;
        pool->queue_max_size=queue_max_size;

        /*根据最大线程上限数 ，给工作线程数组开辟空间，并清零 */
        pool->threads=(pthread_t *)malloc(sizeof(pthread_t)*max_thr_num);
        if(pool->threads == NULL)
        {
            printf("malloc threads fail ");
            break;
        }
        memset(pool->threads,0,sizeof(pthread_t)*max_thr_num);

        /* 队列开辟空间 */
        pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t)*max_thr_num);
        if(pool->task_queue == NULL)
        {
            printf("malloc task_queue fail");
            break;
        }

        /* 初始化互斥锁、条件变量*/
        if(pthread_mutex_init(&(pool->lock),NULL) !=0
            ||pthread_mutex_init(&(pool->thread_counter),NULL) != 0
                ||pthread_cond_init(&(pool->queue_not_empty),NULL) !=0
                ||pthread_cond_init(&(pool->queue_not_full),NULL) !=0)
                {
                    printf("init the lock or cond fail");
                    break;
                }

        /* 启动min_thr_num 个work thread  */
        for(i=0;i<min_thr_num;i++)
        {
            info_t *info=(info_t *)malloc(sizeof(info_t));
            info->pool=pool;
            info->index=i;
            pthread_create(&(pool->threads[i]),NULL,threadpool_func,(void *)info); //创建线程，pool指向当前线程池
            printf("start 0x%X.....threads[%d]\n",(unsigned int)pool->threads[i],i);
            usleep(1000);
        }
        pthread_create(&(pool->adjust_tid),NULL,adjust_func,(void *)pool);       //启动管理者线程
        
        return pool;

    }while(0);
    threadpool_free(pool);    //前面代码调用失败时，释放poll存储空间
    return NULL;
}


/* 模拟处理业务 */

void *process(void *arg)
{
    info_task *info=(info_task *)arg;
    printf("thread 0x%X working on task %d----threads[%d]\n",(unsigned int)pthread_self(),*(int *)info->task_index,info->pthread_index);
    sleep(rand()%10+2);
    printf("task %d is end--------thread[%d]\n",*(int *)info->task_index,info->pthread_index);

    return NULL;
}


int main()
{
    srand(time(NULL));
    /* threadpool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size) */
    threadpool_t *thp =threadpool_create(3,100,100);  //创建线程池，池里最小3个线程，最大100，任务队列最大100
    printf("pool inited\n");

    /* int *num =(int *)malloc(sizeof(int)*20)  */
    int num[20],i;
    for(i=0;i<20;i++)
    {
        num[i]=i;
        printf("add task %d\n",i);
        threadpool_add(thp,process,(void*)&num[i]);   //向线程池中添加任务
        //usleep(5000);
        sleep(1);
    }
    sleep(10);   //等子线程完成任务
    threadpool_destroy(thp);                      
 
    return 0;
}
