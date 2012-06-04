#include <string>
#include <stdio.h>
#include "ConfigFile.h"

using namespace::std;

int main(int argc, char** argv) {

    ConfigFile conf("test1.config");

    // ival as a long
    long ival = conf["ival"];
    // ival as a double
    double ival_as_double = conf["ival"];

    double dblval = conf["dblval"];
    string name = conf["name"];

    // "not_there" is not in the config file; we
    // can use default values
    int def = conf.get("not_there", -9999);

    vector<long> arr1 = conf["arr1"];
    vector<double> arr2 = conf["arr2"];

    printf("ival:           %ld\n", ival);
    printf("ival as double: %lf\n", ival_as_double);
    printf("dblval:         %.16g\n", dblval);
    printf("name:           '%s'\n", name.c_str());
    printf("def:            %d\n", def);

    conf["add_int"] = 12;
    conf["add_str"] = "added string";
    long add_int = conf["add_int"];
    string add_str = conf["add_str"];

    printf("added int:      %ld\n", add_int);
    printf("added string:   '%s'\n", add_str.c_str());

    for (size_t i=0; i<arr1.size(); i++) {
        printf("  arr1[%lu]: %ld\n", i, arr1[i]);
    }
    for (size_t i=0; i<arr2.size(); i++) {
        printf("  arr2[%lu]: %lf\n", i, arr2[i]);
    }

    // now add data from a new config file, over-riding
    // existing entries and adding new ones as neccesary
    conf.load("test2.config");
    ival = conf["ival"];
    dblval = conf["dblval"];
    double x = conf["x"];

    printf("ival (override):    %ld\n", ival);
    printf("dblval:             %.16g\n", dblval);
    printf("x:                  %.16g\n", x);

}
