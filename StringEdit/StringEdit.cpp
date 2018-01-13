#include <iostream.h>


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
		cout << endl << "Enter index (0-38), 44 for a terminator, 55 for a space, or -1 to quit: ";

		int index;
		cin >> index;

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
