#include <stdio.h>

int main()
{
  int i;
  int input;
  int sum_odds = 0;
  int sum_evens = 0;

  for(i=0;i<3;i++)
  {
    printf("Enter a digit: ");
    scanf("%d", &input);

    if ( (input%2) == 0 )
    {
      sum_evens += input;
    }
    else
    {
      sum_odds += input;
    }
  }

  printf("Sum of evens: %d\n", sum_evens);
  printf("Sum of odds: %d\n", sum_odds);
}
