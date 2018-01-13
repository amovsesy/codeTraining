#include <iostream>	// required for 'cout' & 'cin'
#include <conio.h>	// required for 'getch'

using namespace std;	// activates the standard namespace


// function prototypes
float GetFahrenheit();
float Fahrenheit2Celsius(float f);
void DisplayOutput(float f, float c);


int main()
{
	float fahrenheit = GetFahrenheit();

	float celsius = Fahrenheit2Celsius( fahrenheit );

	DisplayOutput( fahrenheit, celsius );


	cout << endl << "Press any key" << endl;
	getch();

	return 0;
}

float GetFahrenheit()
{
	cout << "Enter degrees in Fahrenheit: ";

	float f;
	cin >> f;

	return f;
}

float Fahrenheit2Celsius(float fahrenheit)
{
	return (5.0f / 9.0f) * (fahrenheit - 32.0f);
}

void DisplayOutput(float f, float c)
{
	cout << f << " degrees Fahrenheit is " << c << " Celsius" << endl;
}

