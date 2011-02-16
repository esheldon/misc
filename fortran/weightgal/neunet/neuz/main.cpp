#include <iostream>
#include <sys/time.h>
#include "nr.h"
#include "neu_net.h"
#include "nr_mod.h"

using namespace std;

int main(int argc, char **argv) {

  const double SMALL = 1.e-12;
  const int MAX_IT = 300;

  const double wbeta = 0.0;

  char train_file[256];
  char valid_file[256];
  char weightfile[256];

  if (argc == 4) {
    strcpy(train_file, argv[2]);
    strcpy(valid_file, argv[3]);
    cout << "training using " << train_file << " and " << valid_file << endl;
  } else if (argc == 5) {
    strcpy(train_file, argv[2]);
    strcpy(valid_file, argv[3]);
    strcpy(weightfile, argv[4]);
    cout << "training using " << train_file << " and " << valid_file << ", using weight file " << weightfile << endl;
  } else {
    cerr << "usage: " << argv[0] << "paramfile train_file valid_file <weight_file>" << endl;
    exit(1);
  }
  int NL = 0;

  neu_net *net = new neu_net;
  neu_net *valid_net = new neu_net;

  net->read_param(argv[1]);
  valid_net->read_param(argv[1]);

  net->initialize();
  valid_net->initialize();

  //decide what the weights should be
  if (argc <= 4) {
    net->randomizeWeights();
  } else if (argc == 5) {
    net->read_weights(weightfile);
  }
  net->set_wbeta(wbeta);

  
  valid_net->set_wbeta(wbeta);

  Vec_INT layer_size = net->get_nlayersize();

  cout << "net->initialize success!!" << endl;


  int check = net->read_trainfile(train_file);
  if (check != 0) {
    cerr << "error reading train file" << endl;
    exit(1);
  }
  
  check = valid_net->read_trainfile(valid_file);
  if (check != 0) {
    cerr << "error reading train file" << endl;
    exit(1);
  }

  timeval *cu1 = new timeval;
  gettimeofday(cu1, NULL);
  long seed = cu1->tv_usec;
  //really randomize
  double dd = 0.0;
  for (int i = 0; i < 10000; ++i) {
    dd = ran2(&seed);
  }

  int NNNN = 0;
  NL = net->get_NL();
 
  if (argc <= 4) {
  } else if (argc == 5) {
    layer_size = net->get_nlayersize();
  }
  for (int i = 0; i < NL-1; ++i) {
    NNNN += layer_size[i]*layer_size[i+1];
  }
  cout << "NL = " << NL << endl;
  cout << "Number of w's = " << NNNN << endl;
  //cout << "seed = " << seed << endl;

  Vec_DP winit(NNNN);
  Vec_DP dwinit(NNNN);
  Vec_DP dwout(NNNN);

  Vec_DP bestw(NNNN);

  /*
  for (int i = 0; i < 10000; ++i) {
    double dd = ran2(&seed);
  }

  if (argc <= 4) {
    for (int i = 0; i < NNNN; ++i) {
      double dd = ran2(&seed);
      winit[i] = (2.0*dd - 1.0)/1.0 - 0.0;
      //cout << "winit[" << i << "] = " << winit[i] << ", dd = " << dd << endl;
    }
    net->set_weights(winit);
  } else if (argc == 5) {
    net->get_weights(winit);
  }
  */
  //cout << "blah" << endl;
  net->get_weights(winit);

  net->dE(winit, dwinit);
  DP gtol = SMALL;
  int iter;
  DP fret;
  //cout << "dfpmin running....." << endl;
  int end_i = MAX_IT;
  double best_s = 1.0e18;
  char filename[256];
  strcpy(filename, "blah.wts");
  timeval *t1 = new timeval();
  timeval *t2 = new timeval();
  for (int i = 0; i < MAX_IT; ++i) {
    gettimeofday(t1, NULL);
    dfpmin(winit, gtol, iter, fret, net);
    net->set_weights(winit);
    net->calc_zphot();
    valid_net->set_weights(winit);
    valid_net->calc_zphot();
    double s = valid_net->z_scatter();
    double st = net->z_scatter();
    gettimeofday(t2, NULL);
    double sec = t2->tv_sec - t1->tv_sec + 1.0e-6 * (t2->tv_usec - t1->tv_usec);
    if (s < best_s) {
      best_s = s;
      bestw = winit;
      net->set_weights(bestw);
      net->write_weights(filename);
      cout << "iteration " << i << " done.  ctime = " << sec << " s, iter = " << iter << ", tscatter = " << st << ", zscatter = " << s << "  (best so far)" << endl;
    } else 
      cout << "iteration " << i << " done.  ctime = " << sec << " s, iter = " << iter << ", tscatter = " << st << ", zscatter = " << s << endl;

    if (iter == 0) {
      //randomly perturb the weights and continue;
      /*
      for (int j = 0; j < NNNN; ++j) {
	winit[i] += (winit[i] * (double)(ran2(&seed)*1.0e-7));
      }
      */
      end_i = i;
      break;
    }
  }
  
  strcpy(filename, "zphot.dat");
  valid_net->set_weights(bestw);
  valid_net->calc_zphot();
  valid_net->write_z(filename);
  if (layer_size[NL-1] == 2) {
    strcpy(filename, "sphot.dat");
    net->write_sed(filename);
  }
  
  if (end_i == 0) 
    return 1;
  return 0;
}
