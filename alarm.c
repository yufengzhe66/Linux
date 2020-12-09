#include<stdio.h>
#include<unistd.h>

int main()
{
  int i;
  alarm(1);
  i=0;
  while(1)
  {
    printf("%d\n", i++);
  }
  return 0;
}
