#include <stdio.h>

int main()
{
  int a, i, j;
  
  printf("Please enter a number: ");
  scanf("%d", &a);
  
  for(i=0;i<a;i++)
  {
    for(j=0;j<a;)
    {
      printf("%4d", ++j);
    }
    printf("\n");
  }

  return 0;
}
