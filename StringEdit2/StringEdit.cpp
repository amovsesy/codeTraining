#include <stdlib.h>
#include <iostream.h>
#include <conio.h>



// This function works uses 'getch' instead of 'cin' to allow selective input.
// A loop is used to process each character, but non-numeric chars are ignored.
// The loop runs until the user presses ENTER. 
// Negative values are _not_ supported (the '-' characters is ignored)
// Notice that supporting backspace is a bit of a pain...
int GetInteger()
{
	char str[11];	// just large enough to store MAXINT, plus a NULL
	char ch;		// store each character retrieved via getch
	int i=0;		// the number of valid (numeric) chars entered

	bool done = false;
	while (!done)
	{
		ch = getch();	// get one char...

		if (ch >= '0' && ch <='9') // if it's a number, add it to 'str'
		{
			str[i++] = ch;
			cout << ch;
			cout.flush();
		}
		else if (ch == 8)	// if it's BACKSPACE:
		{
			cout.put( (char)8 );	// back up
			cout << " ";			// clobber the character
			cout.flush();			// force the display of the space
			cout.put( (char)8 );	// back up again (because the previous line advanced the cursor
			i--;					// "forget" the last character added to 'str'
		}
		else if (ch == 13)			// 13 == ENTER
		{
			cout << endl;
			done = true;
		}
	}

	if (i > 0)		// if the user entered something:
	{
		str[i] = 0;			// add a NULL
		return atoi(str);	// convert the string to an integer (atoi is declared in stdlib.h,
							// and converts the numeric chars in an array to an integer
	}

	return -1;		// if the user didn't enter anything, return -1
}



int main()
{
	char str[40] = "initial string - declared char str[40]";

	bool done = false;
	while ( !done )
	{
		cout << endl;
		cout << "------------- string contents --------------------" << endl;
		cout << str << endl;
		cout << "--------------------------------------------------" << endl;
		cout << endl << "Enter index (0-38), 44 for a terminator, 55 for a space, or ENTER to quit: ";
		cout.flush();

		int index;

		// cin >> index;  // this doesn't provide any error checking...
		index = GetInteger();	// GetInteger is our custom solution 

		if (index == -1)
			done = true;
		else if (index >= 0 && index < 39)
		{
			cout << "enter new character: ";
			char ch;
			cin >> ch;
			str[index] = ch;
		}
		else if (index == 44)
		{
			cout << "Enter index for terminator: ";
			int terminatorIndex;
			cin >> terminatorIndex;
			if (terminatorIndex >=0 && terminatorIndex < 39)
				str[terminatorIndex] = 0;
			else
				cout << "Invalid index for terminator - string not modified" << endl;
		}
		else if (index == 55)
		{
			cout << "Enter index for space: ";
			int spaceIndex;
			cin >> spaceIndex;
			if (spaceIndex >=0 && spaceIndex < 39)
				str[spaceIndex] = ' ';
			else
				cout << "Invalid index for space - string not modified" << endl;
		}
		else
			cout << "invalid index" << endl;

	};

	return 0;
}
