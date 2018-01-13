#include <iostream> 
#include <conio.h>

using namespace std;	// activates the standard namespace



int main()
{

	bool done = false;
	while ( ! done )
	{
		cout << "Enter a character, and press [ENTER] (enter 'Q' to quit) ";

		char ch;
		cin >> ch;
		int ascii = ch;

		if (ch == 'Q')
			done = true;
		if (ch >= 'a' && ch <='z')
			cout << " '" << ch << "' (" << ascii << ")  lower-case letter" << endl;
		else if (ch >= 'A' && ch <='Z')
			cout << " '" << ch << "' (" << ascii << ")  upper-case letter" << endl;
		else if (ch >= '0' && ch <= '9')
			cout << " '" << ch << "' (" << ascii << ")  numeric digit" << endl;
		else
			cout << " '" << ch << "' (" << ascii << ")  other" << endl;

		cout << endl;

	}

	return 0;
}

