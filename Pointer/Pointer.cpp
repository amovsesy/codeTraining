#include <ostream.h>
#include <conio.h>


int main()
{
	int var = 33;

	cout << "var is initialized to " << var << endl;

	int* pVar = &var;

	cout << "pVar is assigned to point to var (memory address " << pVar << ")" << endl;

	int var2 = *pVar;

	cout << "var2 is is assigned to *pVar (var2 is now " << var2 << ")" << endl;

	*pVar = 200;

	cout << "*pVar is assigned to 200" << endl;

	cout << "final result: var is " << var << ", var2 is " << var2 << endl;


	cout << endl << "Press any key" << endl;
	getch();

	return 0;
}

