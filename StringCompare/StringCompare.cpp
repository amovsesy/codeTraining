#include <conio.h>
#include <iostream.h>


// STRINGCOMPARE exercise:
// Implmement the OurStringCompare function.
// It must have the following prototype:

int OurStringCompare(char* str1, char* str2);


int main()
{
	char str1[] = "abc";
	char str2[] = "bac";
	char str3[] = "abc";


	int compare1 = OurStringCompare( str1, str2 );
	int compare2 = OurStringCompare( str1, str3 );
	int compare3 = OurStringCompare( str2, str3 );

	cout << "comparision results (zero means equality in this case):" << endl;
	cout << str1 << " == " << str2 << "? " << compare1 << endl;
	cout << str1 << " == " << str3 << "? " << compare2 << endl;
	cout << str2 << " == " << str3 << "? " << compare3 << endl;


	cout << "press any key" << endl;
	getch();

	return 0;
}

