#include <iostream>		// cin && cout
#include <conio.h>		// required for 'getch'
#include <stdlib.h>		// required for 'rand'

using namespace std;	// activates the standard namespace


int main()
{
	for (int i=0; i<10; i++)
	{
		for (int j=0; j<10; j++)
		{
			int r = rand() % 100;

			// format output with wo spaces between values
			cout.width( 2 );	

			cout << r << "  ";
		}
		cout << endl;
	}


	cout << endl << "Press any key" << endl;
	getch();

	return 0;
}

