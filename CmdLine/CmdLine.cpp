#include <conio.h>
#include <ostream.h>
#include <string>
#include <assert.h>



using namespace std;
const int MAX_ARGS = 20;


class CmdLine
{
public:
	CmdLine(int argc, char** argv)
	{
		argCount = argc - 1;
		argList = new string[argCount];

		for (int i=0; i<argCount; i++)
			argList[i] = argv[i+1];

		fullArgCount = 0;
		for (i=0; i<argCount; i++)
		{
			FullArg* fa = new FullArg;
			fa->cmdIndex = i;
			fa->subArgs = 0;
			fullArgList[fullArgCount++] = fa;

			const char* thisArg = argList[i].data();
			if (IsCommandChar( *thisArg ) )
			{
				for (int j=i+1; j<argCount; j++)
				{
					const char* nextArg = argList[j].data();
					if ( ! IsCommandChar( *nextArg ) )
						fa->subArgs++;
					else
						break;
				}

				i += fa->subArgs;								
			}	
		}
	}

	int GetArgCount()	{ return argCount; }

	const string& GetArg(int arg)
	{
		assert(arg>=0);
		assert(arg<argCount);
		return argList[arg];
	}

	const char* GetArgChars(int arg)
	{
		assert(arg>=0);
		assert(arg<argCount);
		return argList[arg].data();
	}

	bool CommandIsPresent(const char* command)
	{
		const char* cmd = GetCommandStr( command );
		for (int i=0; i<fullArgCount; i++)
		{
			int index = fullArgList[i]->cmdIndex;

			const char* p = GetCommandStr(argList[index].data());

			if (strcmp( cmd, p ) == 0)
				return true;
		}
		return false;
	}

	int GetCommandCount(const char* cmd)
	{
		for (int i=0; i<fullArgCount; i++)
		{
			int index = fullArgList[i]->cmdIndex;
			const char* p = GetCommandStr(argList[index].data());
			if (strcmp( cmd, p ) == 0)
				return fullArgList[i]->subArgs;
		}
		return 0;
	}

	const char* GetCommandParam(const char* command, int arg)
	{
		const char* cmd = GetCommandStr( command );
		for (int i=0; i<fullArgCount; i++)
		{
			int index = fullArgList[i]->cmdIndex;
			const char* p = GetCommandStr(argList[index].data());

			if (strcmp( cmd, p ) == 0)
			{
				if (fullArgList[i]->subArgs >= arg)
					return argList[index+arg+1].data();
			}
		}
		return 0;
	}

	void DisplayAll()
	{
		cout << "Raw args (" << argCount << "):" << endl;
		for (int i=0; i<argCount; i++)
		{
			cout << argList[i].data() << endl;
		}
	}

	void DisplayAllFull()
	{
		cout << "Full args (" << fullArgCount << "):" << endl;

		for (int i=0; i<fullArgCount; i++)
		{
			int index = fullArgList[i]->cmdIndex;
			int subs = fullArgList[i]->subArgs;
			cout << argList[index].data();
			for (int j=1; j<=subs; j++)
				cout << " " << argList[index+j].data();
			cout << endl;
		}		
	}

private:

	const char* GetCommandStr(const char* cmd)
	{
		if (IsCommandChar( *cmd ))
			return cmd+1;
		return cmd;
	}

	bool IsCommandChar(char ch)
	{
		if (ch=='-' || ch=='/')
			return true;
		return false;
	}

private:
	string* argList;
	int argCount;

	struct FullArg
	{
		int cmdIndex;
		int subArgs;
	};
	
	FullArg* fullArgList[MAX_ARGS];
	int fullArgCount;
};

int main(int argc, char** argv)
{
	CmdLine cmd( argc, argv );


	cout << "raw aguments: " << endl;
	for (int i=0; i<cmd.GetArgCount(); i++)
	{
		const string& s = cmd.GetArg(i);
		cout << i << ": " << s.data() << endl;
	}

	cout << "------------------" << endl;

	int load = 10;			// default load 
	int iterations = 1;		// default iterations

	cout << "default load: " << load << endl;
	cout << "default iterations: " << iterations << endl;


	cout << endl << "checking command-line argments..." << endl << endl;

	int paramCount = cmd.GetCommandCount( "load" );
	if (paramCount > 0)
	{
		const char* loadStr = cmd.GetCommandParam( "load", 0 );
		load = atoi( loadStr );	// override load
	}

	paramCount = cmd.GetCommandCount( "iterations" );
	if (paramCount > 0)
	{
		const char* iterationsStr = cmd.GetCommandParam( "iterations", 0 );
		iterations = atoi( iterationsStr );	// override iterations
	}
	
	cout << "final load: " << load << endl;
	cout << "final iterations: " << iterations << endl;


	cout << endl << "press any key" << endl;
	getch();

	return 0;

};
