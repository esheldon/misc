#include "KDTree.h"
#include "Catalog.h"
#include "weights.h"

// NDIM defined in params.h
#include "params.h"

/*
 * Procedure:  
 *
 * For each training set object i located at p(i), get the distance d to the
 * nth nearest training set neighbor.  Then find the number of /photometric/
 * objects n_p within distance d from the same point p(i) in space.  From
 * this generate a "weight" for the training object
 *
 * n_near = number of nearest neighbors
 * not normalized weight:
 *    wu_i = n_p/n_near
 * normalized weight
 *    w_i = wu_i/total(wu_i)
 * 
 */

void get_train_weights(struct Catalog& train, struct Catalog& photo, int n_near) {
  KDTree<NDIM> kd_train(train.points);
  KDTree<NDIM> kd_photo(photo.points);

  double total_weights = 0.0;

  photo.num.resize(photo.points.size());

  size_t ntrain = train.points.size();
  int step=10000;
  if (ntrain > step) cout<<"Each dot is "<<step<<"\n";

  for (size_t i=0; i<ntrain; i++) {

      // will get distance to closest n_near neighbors in the train sample
      // the zeroth element will hold the farthest
      vector<double> ds(n_near);
      // also  the indices
      vector<int> ns(n_near);

      kd_train.nneigh(i, ds, ns);

      // listp will hold indices of photo poits within distance ds[0]
      // of the training point.
      vector<int> listp;
      kd_photo.pointsr(ds[0], train.points[i], listp);

      int n=listp.size();
      train.weights[i] = double(n)/double(n_near);
      total_weights += train.weights[i];

      // count how many times each data object was used
      for (size_t j=0; j<n; j++) {
          photo.num[ listp[j] ]++;
      }

      if ( (ntrain > step) && (i % step) == 0 ) {
          cout<<".";
          flush(cout);
      }
  }
  if (ntrain > step) {
      cout<<"\n";flush(cout);
  }

  // normalize weights
  for (size_t i=0; i<train.points.size(); i++) {
      train.weights[i] /= total_weights;
  }

}


