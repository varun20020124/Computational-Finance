#include <iostream>
#include "bond.h"

using namespace std;

int main()
{
    Bond default_bond;

    cout << "Default Bond:" << endl;
    cout << default_bond.ToString() << endl;

    Bond bond_2035("11/19/2035", 0.5, 0.07);

    cout << endl;
    cout << "2035 Bond:" << endl;
    cout << bond_2035.ToString() << endl;

    return 0;
}