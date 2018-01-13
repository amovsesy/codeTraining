#include <conio.h>
#include <iostream.h>


// RECTTEST Exercise:
// Implement the PointInRect function


struct Point 
{ 
	long x; 
	long y; 
}; 


struct Rectangle 
{ 
	long left; 
	long top; 
	long right; 
	long bottom; 
}; 

void DisplayRect(Rectangle);
void DisplayPoint(Point);

// prototype of function that you need to write
bool PointInRect(Rectangle rect, Point point);


int main()
{
	Rectangle rect[] =
	{
		{ 0  , 0  , 100, 100 },
		{ 45, 45  , 145, 145 },
		{ -100, 45, 200, 55 },
	};
	int rectCount = sizeof(rect)/sizeof(Rectangle);

	Point point[] =
	{
		{ -5, 50 },
		{ 50, -5 },
		{ 105, 50 },
		{ 50, 105 },
		{ 50, 50  },
		{ 105, 105 },
	};
	int pointCount = sizeof(point)/sizeof(Point);


	for (int r=0; r<rectCount; r++)
	{
		cout << "using rect " << r+1 << " ";
		DisplayRect( rect[r] );
		cout << endl;

		for (int p=0; p<pointCount; p++)
		{
			bool pointInRect = PointInRect( rect[r], point[p] );

			cout << "point " << p+1 << ": ";
			DisplayPoint( point[p] );
			if (pointInRect) 
				cout << "  true" << endl;
			else
				cout << "  false" << endl;


		}
		cout << endl;
	}


	cout << "press any key";
	getch();


	return 0;
}

void DisplayRect(Rectangle r)
{
	cout << "{" << r.left << "," << r.top << "," << r.right << "," << r.bottom << "}";
}

void DisplayPoint(Point p)
{
	cout << "{" << p.x << "," << p.y << "}";
}


