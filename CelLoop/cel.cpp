#include <iostream>

using namespace std;

int main()
{
  char m;
  double f;
  double c;

  while (true)
  {
    cout << "Enter a degree in farinheight: ";
    cin >> f;

    c = (5.0/9.0) * (f-32.0);

    cout << endl << f << " is equal to " << c. << " degrees celcius" << endl;

    cout << endl << "Would you like to enter another temperature?(y for yes, no for no): ";
    cin >> m;

    if(m == 'n' || m == 'N')
      break;
  }

  return 0;
}