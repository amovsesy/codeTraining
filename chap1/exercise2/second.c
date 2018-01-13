#include <stdio.h>

int main()
{
  int num1, num2;
  float sum;

  fprintf(stderr, "Enter the 1st num: ");
  scanf("%d", &num1);
  fprintf(stderr, "Enter the 2st num: ");
  scanf("%d", &num2);

  sum = num1 + num2;
  printf("The average is %.1f.\n", sum/2);

  return 0;
}
