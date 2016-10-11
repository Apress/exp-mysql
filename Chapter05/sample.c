#include <stdio.h>
#include <stdlib.h>

static int factorial(int num)
{
  int i;
  int fact = 1;

  for (i = 1; i < num; i++)
  {
    fact += fact * i;
  }
  return fact;
}

int main(int argc, char *argv[])
{
  int num;
  int fact = 0;

  num = atoi(argv[1]);
  fact = factorial(num);
  printf("%d! = %d\n", num, fact);
  return 0;
}

