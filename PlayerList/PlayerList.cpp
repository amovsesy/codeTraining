#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <conio.h>

using namespace std;

struct Player
{
	char name[80];
	int score;
};


void LoadPlayerInfo(char* filename, Player* playerArray, int& numPlayers, int maxPlayers);
void DisplayPlayerInfo(Player* playerArray, int numPlayers);
char* OurStrCpy(char* dst, char* src);


int main()
{
	Player player[10];
	int numPlayers = 0;


	cout << "loading player info..." << endl << endl;

	LoadPlayerInfo( "playerlist.txt", player, numPlayers, 10 );

	DisplayPlayerInfo( player, numPlayers );

	cout << endl << "Press any key" << endl;
	getch();

	return 0;
}

void LoadPlayerInfo(char* filename, Player* playerArray, int& numPlayers, int maxPlayers)
{
	fstream str( filename, ios::in );

	while (numPlayers < maxPlayers && !str.eof() )

	if (str.is_open())
	{
		char name[80];
		str.getline( name, 80 );

		if ( !str.eof() )
			OurStrCpy( playerArray[numPlayers].name, name );

		char score[10];
		str.getline( score, 10 );

		if ( !str.eof() )
		{
			playerArray[numPlayers].score = atoi(score);
			numPlayers++;
		}
	}

}

void DisplayPlayerInfo(Player* playerArray, int numPlayers)
{
	Player* p = playerArray;

	for (int i=0; i<numPlayers; i++)
	{
		cout << p->name << "  " << p->score << endl;
		p++;
	}
}

char* OurStrCpy(char* dst, char* src)
{
	char* ret = dst;

	while (*src)
	{
		*dst++ = *src++;
	}

	*dst = 0;

	return ret;
}

