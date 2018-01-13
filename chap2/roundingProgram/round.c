#include <stdio.h>

int main()
{
  int x;
  float y;

  printf("Please enter a floating point number: ");
  fflush(stdout);
  scanf("%f", &y);
  x = (int)(y + 0.5);
  printf("The number was %f and rounded is %d\n", y, x);
  return 0;
}
