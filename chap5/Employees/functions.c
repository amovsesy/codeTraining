#include "employee.h"

int nextID = 1;

employeelist* New_List()
{
  employeelist* newlist = (employeelist*)malloc(sizeof(employeelist));

  newlist->first = NULL;
  newlist->last = NULL;
  newlist->length = 0;

  return newlist;
}


employee* New_Employee()
{
  employee* node = (employee*)malloc(sizeof(employee));

  node->employeeID = 0;
  node->first = NULL;
  node->last = NULL;
  node->title = NULL;
  node->salary = 0;
  node->next = NULL;
  node->prev = NULL;

  return node;
}


void Add_Employee(char firstName[], char lastName[], char job[], long money)
{
  employee* lastNode = list->last;
  employee* newNode = New_Employee();

  if ( lastNode == NULL )
  {
    list->first = newNode;
    list->last = newNode;
    list->length = 1;
  }
  else
  {
    list->last = newNode;
    lastNode->next = newNode;
    newNode->prev = lastNode;
    list->length++;
  }

  newNode->first = (char*)malloc(sizeof(char)*MAX_CHAR_LENGTH);
  newNode->last = (char*)malloc(sizeof(char)*MAX_CHAR_LENGTH);
  newNode->title = (char*)malloc(sizeof(char)*MAX_CHAR_LENGTH);

  strcpy(newNode->first, firstName);
  strcpy(newNode->last, lastName);
  strcpy(newNode->title, job);
  newNode->salary = money;
  newNode->employeeID = nextID++;

  printf("\n{%d, %s, %s, %s, %ld} was added.\n\n", newNode->employeeID, newNode->first, newNode->last, newNode->title, newNode->salary);
}


void Remove_Employee(employee* remove)
{
  employee* nodeToRemove = remove;

  printf("\n{%d, %s, %s, %s, %ld} was removed.\n\n", 
    nodeToRemove->employeeID, nodeToRemove->first, nodeToRemove->last, nodeToRemove->title, nodeToRemove->salary);

  if ( nodeToRemove->prev != NULL && nodeToRemove->next == NULL )
  {
    list->last = nodeToRemove->prev;
    nodeToRemove->prev->next = NULL;
  }
  else if ( nodeToRemove->prev != NULL )
  {
    nodeToRemove->prev->next = nodeToRemove->next;
    nodeToRemove->next->prev = nodeToRemove->prev;
  }
  else if ( list->length == 1 )
  {
    list->first = NULL;
    list->last = NULL;
  }
  else
  {
    list->first = nodeToRemove->next;
    nodeToRemove->next->prev = NULL;
  }

  list->length--;
  free(nodeToRemove->first);
  free(nodeToRemove->last);
  free(nodeToRemove->title);
  free(nodeToRemove);
}


void Edit_Employee(employee* node)
{
  int input;
  char name[MAX_CHAR_LENGTH];

  while(1)
  {
    printf("\nWhat would you like to edit?\nEnter -\n");
    printf("1 to edit the employee's first name\n2 to edit the employee's last name\n");
    printf("3 to edit the employee's title\n4 to edit the employee's salary: ");
    scanf(" %d", &input);

    if ( input == 1 )
    {
      printf("Enter the new first name: ");
      scanf(" %s", name);
      strcpy(node->first, name);
    }
    else if ( input == 2 )
    {
      printf("Enter the new last name: ");
      scanf(" %s", name);
      strcpy(node->last, name);
    }
    else if ( input == 3 )
    {
      printf("Enter the new title: ");
      scanf(" %s", name);
      strcpy(node->title, name);
    }
    else if ( input == 4 )
    {
      printf("Enter the new salary: ");
      scanf(" %d", &input);
      node->salary = input;
    }
    else
    {
      printf("You have entered an invalid operation\n");
    }

    printf("Would you like to edit anything else?\nEnter -\n1 for yes\n2 for no: ");
    scanf(" %d", &input);

    if ( input == 2 )
    {
      printf("\n{%d, %s, %s, %s, %ld} was changed\n\n", node->employeeID, node->first, node->last, node->title, node->salary);
      break;
    }
  }
}


employee* Find_By_Name(char firstName[], char lastName[])
{
  employee* node;

  for(node=list->first; node!=NULL; node=node->next)
  {
    if ( !strcmp(node->first, firstName) && !strcmp(node->last, lastName) )
    {
      break;
    }
  }

  return node;
}


employee* Find_By_ID(int IDnum)
{
  employee* node;

  for(node=list->first; node!=NULL; node=node->next)
  {
    if ( node->employeeID == IDnum )
    {
      break;
    }
  }

  return node;
}

void Print_List()
{
  employee* node;
  char *s0 = "ID";
  char *s1 = "First Name";
  char *s2 = "Last Name";
  char *s3 = "Job Title";
  char *s4 = "Salary";

  printf("\n\n%-5s%-20s%-20s%-20s%-10s\n", s0, s1, s2, s3, s4);

  for(node=list->first; node!=NULL; node=node->next)
  {
    printf("%-5d%-20s%-20s%-20s%-10d\n", node->employeeID, node->first, node->last, node->title, node->salary);
  }

  printf("\n");
}


void Print_To_File()
{
  employee* node;
  char *s0 = "ID";
  char *s1 = "First Name";
  char *s2 = "Last Name";
  char *s3 = "Job Title";
  char *s4 = "Salary";
  FILE* fp;


  if ( (fp=fopen("Employees.csv", "w")) == NULL )
  {
    printf("There was an error opening the file\n");
  }

  /*if ( (fp=fopen("Employees.txt", "w")) == NULL )
  {
    printf("There was an error opening the file\n");
  }

  fprintf(fp, "%-5s%-20s%-20s%-20s%-10s\n", s0, s1, s2, s3, s4);*/
  fprintf(fp, "%s,%s,%s,%s,%s\n", s0, s1, s2, s3, s4);

  for(node=list->first; node!=NULL; node=node->next)
  {
    //fprintf(fp, "%-5d%-20s%-20s%-20s%-10d\n", node->employeeID, node->first, node->last, node->title, node->salary);
    fprintf(fp, "%d,%s,%s,%s,%d\n", node->employeeID, node->first, node->last, node->title, node->salary);
  }

  fclose(fp);
}


void Get_Add_Data()
{
  long money = 0;
  char firstname[MAX_CHAR_LENGTH];
  char lastname[MAX_CHAR_LENGTH];
  char position[MAX_CHAR_LENGTH];

  printf("Enter the employee's first name: ");
  scanf(" %s", firstname);
  printf("Enter %s's last name: ", firstname);
  scanf(" %s", lastname);
  printf("Enter %s %s's job title: ", firstname, lastname);
  scanf(" %s", position);
  printf("Enter %s %s's yearly salary: ", firstname, lastname);
  scanf(" %ld", &money);

  Add_Employee(firstname, lastname, position, money);
}