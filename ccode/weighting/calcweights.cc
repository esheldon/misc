#include "Catalog.h"
#include "weights.h"
#include "util.h"

int main(int argc, char **argv) {

  if (argc < 6) {
      cout<<"Usage: \n";
      cout<<"   calcweights trainfile photfile n_near weightfile numfile\n\n";

      cout<<"     weightsfile and numfile are outputs\n";
      cout<<"     n_near=5 is typical first run, 100 second run\n";
      cout<<"     don't forget to remove weight=0 objects for second run\n";
      return(1);
  }

  string trainfile = argv[1];
  string photofile = argv[2];
  //int n_near = charstar2int(argv[3]);
  int n_near = converter<int>(argv[3]);
  string weightsfile = argv[4];
  string numfile = argv[5];

  cout<<"number of nearest neighbors: "<<n_near<<"\n";

  struct Catalog train(trainfile,"train");
  struct Catalog photo(photofile,"photo");

  // this will set the values of the weights vector in train
  get_train_weights(train, photo, n_near);

  train.write(weightsfile);
  photo.write_num(numfile);

}


