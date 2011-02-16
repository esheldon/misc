#include <iostream>
#include "corrobj.h"

using namespace std;

int main(int argc, char *argv[])
{

    if (argc < 2) 
    {
        cout << "-Syntax: correlate par_file" << endl;
        return(1);
    }

    char *parfile = argv[1];

    // Initialize the object
    corrobj corr(parfile);

    corr.correlate();

    return(0);
}


