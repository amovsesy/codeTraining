#include <ostream.h>	// required for 'cout'
#include <conio.h>		// required for 'getch'
#include <stdlib.h>


struct Node
{
	int value;
	Node* next;
};

Node* BuildSampleList()
{
	Node* head = new Node;
	Node* next = new Node;
	Node* tail = new Node;

	head->value = 1;
	head->next = next;

	next->value = 2;
	next->next = tail;

	tail->value = 3;
	tail->next = 0;

	return head;
}

void ShowAll(Node* head)
{
	int count = 0;
	Node* current = head;

	while (current)
	{
		cout << current->value << " ";
		count++;
		current = current->next;
	}

	cout << "  (" << count << ")" << endl;
}

int GetLength(Node* head)
{
	int count = 0;
	Node* current = head;

	while (current)
	{
		count++;
		current = current->next;
	}

	return count;
}

void AddToTail(Node* head, int value)
{
	int count = 0;
	Node* current = head;

	while (current->next)
	{
		current = current->next;
	}

	Node* p = new Node;
	p->value = value;
	p->next = 0;

	current->next = p;
}

void PushHead(Node*& head, int value)
{
	Node* oldHead = head;

	head = new Node;
	head->value = value;
	head->next = oldHead;
}


int main()
{
	cout << "building sample list..." << endl;
	Node* list = BuildSampleList();
	ShowAll(list);

	cout << "adding 100 to tail..." << endl;
	AddToTail( list, 100 );
	ShowAll(list);

	cout << "pushing 200 to head..." << endl;
	PushHead( list, 200 );
	ShowAll(list);

	cout << endl << "creating random list..." << endl;

	Node* list2 = 0;
	for (int i=0; i<10; i++)
	{
		PushHead( list2, rand()%100 );
	}

	ShowAll( list2 );

	cout << "adding 1000 to tail..." << endl;
	AddToTail( list2, 1000 );

	ShowAll( list2 );


	cout << endl << "press any key" << endl;
	getch();
	
	return 0;
}