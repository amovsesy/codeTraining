#include <stdio.h>

int main()
{
  int input;
  int sum_odds = 0;
  int sum_evens = 0;

  while(1)
  {
    printf("Enter a digit(enter 0 to quit): ");
    scanf("%d", &input);

    if ( input == 0 )
    {
      break;
    }

    switch(input%2)
    {
      case 0:
        sum_evens += input;
        break;
      case 1:
        sum_odds += input;
        break;
      default:
        continue;
    }
  }

  printf("Sum of evens: %d\n", sum_evens);
  printf("Sum of odds: %d\n", sum_odds);
}
