#include "PofZ.h"
#include "util.h"


int main(int argc, char **argv) {
  if (argc < 9) {
      cout<<"Usage: \n";
      cout<<"  calcpofz weightsfile photofile n_near nz ";
      cout<<                            " zmin zmax pzfile zfile\n\n";

      cout<<"    weightsfile is the output from calcweights. \n";
      cout<<"    n_near should be about 100\n";
      cout<<"    nz is the number of z points in the p(z), e.g. 20 or 30\n";
      return(1);
  }

  string wtrainfile = argv[1];
  string photofile = argv[2];
  int n_near = converter<int>(argv[3]);
  int nz = converter<int>(argv[4]);
  double zmin = converter<double>(argv[5]);
  double zmax = converter<double>(argv[6]);

  string pzfile=argv[7];
  string zfile=argv[8];


  cout<<"number of nearest neighbors: "<<n_near<<"\n";
  cout<<"nz:   "<<nz<<"\n";
  cout<<"zmin: "<<zmin<<"\n";
  cout<<"zmax: "<<zmax<<"\n"; flush(cout);

  struct PofZ pz(wtrainfile,
                 n_near,
                 zmin,zmax,
                 nz);

  pz.calc(photofile, pzfile, zfile);

}
