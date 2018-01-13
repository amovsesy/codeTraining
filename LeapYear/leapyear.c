#include <stdio.h>

int isLeapYear(int something);

int main()
{
  int year;

  printf("Enter a year AD, for example, 1997: ");
  scanf(" %d", &year);

  if ( isLeapYear(year) )
  {
    printf("%d is a leap year.\n", year);
  }
  else
  {
    printf("%d is not a leap year.\n", year);
  }

  return 0;
}

int isLeapYear(int something)
{
  if ( something % 4 )
  {
    return 0;
  }
  else if ( something % 100 )
  {
    return 1;
  }
  else if ( something % 400 )
  {
    return 0;
  }

  return 1;
}