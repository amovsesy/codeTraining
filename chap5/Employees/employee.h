#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


#define MAX_LENGTH 31
#define MAX_CHAR_LENGTH 21


typedef struct employeeStruct
{
  int employeeID;
  char *first;
  char *last;
  char *title;
  long salary;
  struct employeeStruct* next;
  struct employeeStruct* prev;
}employee;

typedef struct employeeListStrct
{
  employee* first;
  employee* last;
  int length;
}employeelist;


employeelist* list;


employeelist* New_List();

employee* New_Employee();

void Add_Employee(char firstName[], char lastName[], char job[], long money);

void Remove_Employee(employee* remove);

void Edit_Employee(employee* node);

employee* Find_By_Name(char firstName[], char lastName[]);

employee* Find_By_ID(int IDnum);

void Print_List();

void Print_To_File();

void Get_Add_Data();