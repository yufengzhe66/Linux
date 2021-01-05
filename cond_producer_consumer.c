#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>

void err_thread(int ret, char *str)
{
   if(ret != 0)
    {
      fprintf(stderr,"%s:%s\n",str,strerror(ret));
      pthread_exit(NULL);
    }
}


struct msg
{
  int num;
  struct msg *next;
};


struct msg *head;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;                               //定义初始化互斥量
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;                              //定义条件变量


void *producer(void *arg)
{
  struct msg *mp;
  while(1)
 {
  mp = (struct msg*)malloc(sizeof(struct msg));
  mp -> num = rand() % 100 + 1;                                                  //模拟一个生产数据
  printf("--produce %d\n", mp -> num);
  
  pthread_mutex_lock(&mutex);                                                    //加锁       
  mp -> next = head;
  head = mp;                                                                     //写公共区域
  pthread_mutex_unlock(&mutex);                                                  //解锁互斥量
  pthread_cond_signal(&has_data);                                                //唤醒阻塞在条件变量 has_data上的线程
  sleep(rand() % 3);
 }
  return NULL;
}

void *consumer(void *arg)
{
  struct msg *mp;
  while(1)
 {  
  pthread_mutex_lock(&mutex);
  if(head == NULL)
    {
      pthread_cond_wait(&has_data, &mutex);                                       //阻塞等待条件变量，解锁  pthread_cond_wait返回时，重新加锁 mutex
    }
  
  mp = head;
  head = head -> next;
  printf("----------------consumer %d \n", mp -> num);
  free(mp);
  pthread_mutex_unlock(&mutex);
  sleep(rand() % 3);
  }
  return NULL;
}


int main()
{
  pthread_t pid, cid;

  srand(time(NULL)); 

  int ret = pthread_create(&pid, NULL, producer, NULL);
  if(ret != 0)
    {
      err_thread(ret, "pthread_create producer error");
    }

  
  ret = pthread_create(&cid, NULL, consumer, NULL);
  if(ret != 0)
    {
      err_thread(ret, "pthread_create consumer error");
    }

  pthread_join(pid, NULL);
  pthread_join(cid, NULL);

  return 0;
}
