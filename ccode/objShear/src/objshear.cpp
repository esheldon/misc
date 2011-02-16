#include "LensSource.h"

using namespace std;

int main(int argc, char *argv[])
{

    if (argc < 2) 
    {
        cout << "-Syntax: objshear par_file" << endl;
        return(1);
    }

    string par_file = argv[1];
    LensSource lens_source(par_file);

    // Does what we've read make sense?
    lens_source.TestLcat();
    lens_source.TestScat();
    lens_source.TestRev();

    lens_source.DoTheMeasurement();
    return(0);
}



