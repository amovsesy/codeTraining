#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>


class Player
{
public:
	Player()
	{
		pName = 0;
		score = 0;
	}
	Player(const char* n, int s)
	{
		score = s;

		pName = 0;
		SetName( n );
	}


	void SetScore(int s)  { score = s; }

	int GetScore() { return score; }

	void SetName(const char* n)
	{
		delete [] pName;
		pName = new char[strlen(n) + 1];
		strcpy( pName, n );
	}

	const char* GetName()
	{
		return pName;
	}

	virtual ~Player()
	{
		delete [] pName;
	}

	virtual void ProcessCommands()
	{
		cout << "(base class behavior)" << endl;
	}

private:
	char* pName;
	int score;
}; 


class HumanPlayer : public Player
{
public:
	HumanPlayer(const char* name)
		:	Player( name, 0 )
	{
		cout << GetName() << " (human) created" << endl;
	}

	virtual void ProcessCommands()
	{
		cout << GetName() << " (human) processing commands" << endl;
//		Player::ProcessCommands();	// call the base class version
	}
	
	~HumanPlayer()
	{
		cout << GetName() << " (human) destroyed" << endl;
	}
};

class ArtificialPlayer : public Player
{
public:
	ArtificialPlayer(const char* name)
		:	Player( name, 0 )
	{
		cout << GetName() << " (AI) created" << endl;
	}

	virtual void ProcessCommands()
	{
		cout << GetName() << " (AI) processing commands" << endl;
//		Player::ProcessCommands();	// call the base class version
	}
	
	~ArtificialPlayer()
	{
		cout << GetName() << " (AI) destroyed" << endl;
	}
};

const char* PlayerName[] =
{
	"MegaLoser",
	"Daft Pansy",
	"Incredulous",
	"Tasty Crouton",
};


const int NumPlayers = 4;
Player* player[NumPlayers];

void CreatePlayers()
{
	srand(time(0));

	cout << "------ creating players ------" << endl;
	for (int i=0; i<NumPlayers; i++)
	{
		if (rand()%2)
			player[i] = new HumanPlayer( PlayerName[i] );
		else
			player[i] = new ArtificialPlayer( PlayerName[i] );
	}
	cout << endl;
}

void DestroyPlayers()
{
	cout << "------ destroying players ------" << endl;

	for (int i=0; i<NumPlayers; i++)
		delete player[i];

	cout << endl;
}

int main()
{
	CreatePlayers();

	cout << "----- processing commands -----" << endl;
	for (int i=0; i<NumPlayers; i++)
	{
		player[i]->ProcessCommands();
	}
	cout << endl;

	DestroyPlayers();

	cout << "press any key" << endl;
	getch();

	return 0;
}

