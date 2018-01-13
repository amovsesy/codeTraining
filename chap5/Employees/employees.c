#include "employee.h"

int main()
{
  int i, j;
  int input;
  char firstname[MAX_CHAR_LENGTH];
  char lastname[MAX_CHAR_LENGTH];
  employee* node = NULL;
  
  list = New_List();

  while(1)
  {
    printf("\n\nWhat would you like to do?\n");
    printf("Enter -\n1 to quit\n2 to add a new employees\n3 to delete an employee\n4 to print all current employees\n");
    printf("5 to edit a current employee: ");
    scanf(" %d", &input);

    if ( input == 1 )
    {
      Print_To_File();
      break;
    }
    else if ( input == 2 )
    {
      printf("\nHow many employees would you like to add? ");
      scanf(" %d", &input);

      for(i=0;i<input;i++)
      {
        Get_Add_Data();
      }
    }
    else if ( input == 3 )
    {
      printf("\nHow many employees would you like to delete? ");
      scanf(" %d", &j);
      printf("\nWould you like to delete by name or by employee ID?\n");
      printf("Enter -\n1 to delete by name\n2 to delete by employee ID: ");

      while(1)
      {     
        scanf(" %d", &input);

        if ( input == 1 )
        {
          printf("Enter the employee's first and last name separated by spaces: ");
          for(i=0;i<j;i++)
          {
            scanf(" %s", firstname);
            scanf(" %s", lastname);

            node = Find_By_Name(firstname, lastname);

            if ( node == NULL )
            {
              printf("The record was not found.\n");
            }
            else
            {
              Remove_Employee(node);
            }
          }

          break;
        }
        else if ( input == 2 )
        {
          printf("Enter the ID's of the employee's to remove separated by spaces: ");
          for(i=0;i<j;i++)
          {
            scanf(" %d", &input);

            node = Find_By_ID(input);

            if ( node == NULL )
            {
              printf("The record was not found.\n");
            }
            else
            {
              Remove_Employee(node);
            }
          }

          break;
        }
        else
        {
          printf("You have entered an invalid operation.\n");
          i--;
        }
      }
    }
    else if ( input == 4 )
    {
      Print_List();
    }
    else if ( input == 5 )
    {
      printf("\nWould you like to find the employee by name or by employee ID?\n");
      printf("Enter -\n1 to delete by name\n2 to delete by employee ID: "); 
      scanf(" %d", &input);

      if ( input == 1 )
      {
        while(1)
        {
          printf("Enter the employee's first and last name separated by a space: ");        
          scanf(" %s", firstname);
          scanf(" %s", lastname);

          node = Find_By_Name(firstname, lastname);

          if ( node == NULL )
          {
            printf("The record was not found.\n");
          }
          else
          {
            printf("\n{%d, %s, %s, %s, %ld}\n", node->employeeID, node->first, node->last, node->title, node->salary);
            printf("Is this the employee?\nEnter -\n1 for yes\n2 for no: ");
            scanf(" %d", &input);

            if ( input == 1 )
            {
              Edit_Employee(node);
              break;
            }
            else
            {
              continue;
            }
          }
        }
      }
      else if ( input == 2 )
      {
        while(1)
        {
          printf("Enter the ID of the employee to edit: ");
          scanf(" %d", &input);

          node = Find_By_ID(input);

          if ( node == NULL )
          {
            printf("The record was not found.\n");
          }
          else
          {
            printf("\n{%d, %s, %s, %s, %ld}\n", node->employeeID, node->first, node->last, node->title, node->salary);
            printf("Is this the employee?\nEnter -\n1 for yes\n2 for no: ");
            scanf(" %d", &input);

            if ( input == 1 )
            {
              Edit_Employee(node);
              break;
            }
            else
            {
              continue;
            }
          }
        }
      }
    }
    else
    {
      printf("You have entered an invalid operation.\n");
    }
  }

  return 0;
}