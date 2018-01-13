#include <iostream>		// required for 'cout'
#include <conio.h>		// required for 'getch'

using namespace std;	// activates the standard namespace


int main()
{
	cout << "IntMult sample" << endl;

	short a1 = 123, a2 = 456;
	short a3 = a1 * a2;
	cout << endl << "using short data type:" << endl;
	cout << a1 << " * " << a2 << " = " << a3 << " (incorrect!)" << endl;

	int b1 = 123, b2 = 456;
	int b3 = b1 * b2;
	cout << endl << "using int data type:" << endl;
	cout << b1 << " * " << b2 << " = " << b3 << endl;

	int c1 = 123450, c2 = 67890;
	int c3 = c1 * c2;
	cout << endl << "using int data type:" << endl;
	cout << c1 << " * " << c2 << " = " << c3 << " (incorrect!)" << endl;

	unsigned d1 = 123450, d2 = 67890;
	unsigned d3 = d1 * d2;
	cout << endl << "using unsigned data type:" << endl;
	cout << d1 << " * " << d2 << " = " << d3 << endl;


	cout << endl << "Press any key" << endl;
	getch();

	return 0;
}