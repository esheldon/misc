#include <iostream>
#include <ostream>
#include <limits>

int main()
{

	typedef std::numeric_limits< float > f;
	typedef std::numeric_limits< double > d;
	typedef std::numeric_limits< long double > ld;
	typedef std::numeric_limits< short > s;
	typedef std::numeric_limits< int > i;
	typedef std::numeric_limits< long > l;
	typedef std::numeric_limits< long long > ll;

	using namespace std;



	cout << "float:\n";
	cout << "\tsizeof: \t\t" << sizeof(float) <<endl;
	cout << "\tdigits (bits):\t\t" << f::digits << endl;
	cout << "\tdigits (decimal):\t" << f::digits10 << endl;

	cout << endl;

	cout << "double:\n";
	cout << "\tsizeof: \t\t" << sizeof(double) <<endl;
	cout << "\tdigits (bits):\t\t" << d::digits << endl;
	cout << "\tdigits (decimal):\t" << d::digits10 << endl;

	cout << endl;

	cout << "long double:\n";
	cout << "\tsizeof: \t\t" << sizeof(long double) <<endl;
	cout << "\tdigits (bits):\t\t" << ld::digits << endl;
	cout << "\tdigits (decimal):\t" << ld::digits10 << endl;

	cout << endl;

	cout << "short:\n";
	cout << "\tsizeof: \t\t" << sizeof(short) <<endl;
	cout << "\tdigits (bits):\t\t" << s::digits << endl;
	cout << "\tdigits (decimal):\t" << s::digits10 << endl;

	cout << endl;

	cout << "int:\n";
	cout << "\tsizeof: \t\t" << sizeof(int) <<endl;
	cout << "\tdigits (bits):\t\t" << i::digits << endl;
	cout << "\tdigits (decimal):\t" << i::digits10 << endl;

	cout << endl;

	cout << "long:\n";
	cout << "\tsizeof: \t\t" << sizeof(long) <<endl;
	cout << "\tdigits (bits):\t\t" << l::digits << endl;
	cout << "\tdigits (decimal):\t" << l::digits10 << endl;

	cout << endl;

	cout << "long long:\n";
	cout << "\tsizeof: \t\t" << sizeof(long long) <<endl;
	cout << "\tdigits (bits):\t\t" << ll::digits << endl;
	cout << "\tdigits (decimal):\t" << ll::digits10 << endl;

}
