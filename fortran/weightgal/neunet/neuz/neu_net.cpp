// $Id: neu_net.cpp,v 1.2 2007/02/19 23:13:24 oyachai Exp $
#include "neu_net.h"

neu_net::neu_net() {

}

neu_net::~neu_net() {
  
}

void neu_net::read_param(const char *paramfile) {
  ifstream in(paramfile);
  int NCHAR = 256;
  char *line = new char[NCHAR];
  char *delims = new char[NCHAR];
  char *token;

  //cout << "Reading param file:" << paramfile << endl;

  Vec_INT nll(100);
  strcpy(delims, " ");

  //Required parameters
  int HAS_MAXZ = 0;
  int HAS_MINZ = 0;
  int HAS_NLAYER = 0;
  int HAS_NFILTER = 0;
  int HAS_NTARGET = 0;

  //default values
  max_mag = 40.0;
  min_mag = 10.0;
  max_s = 4.0;
  min_s = -1.0;
  K = 1.0;

  while (!in.eof()) {
    in.getline(line, NCHAR);
    token = strtok(line, delims);
    if (token == NULL)
      continue;
    if (strncmp("ZMIN", token, 4) == 0) {
      HAS_MINZ = 1;
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      min_z = atof(token);
    } else if (strncmp("ZMAX", token, 4) == 0) {
      HAS_MAXZ = 1;
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      max_z = atof(token);
    } else if (strncmp("MMAX", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      max_mag = atof(token);
    } else if (strncmp("MMIN", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      min_mag = atof(token);
    } else if (strncmp("NLAYER", token, 5) == 0) {
      HAS_NLAYER = 1;
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      N_LAYERS = atoi(token);
    } else if (strncmp("NLH1", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      nll[1]= atoi(token);
    } else if (strncmp("NLH2", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      if (N_LAYERS < 4) {
	cerr << "Error: invalid option in .param file" << endl;
	cerr << "       NLH2 can only be used if NLAYER > 3" << endl;
	exit(10);
      }
      nll[2] = atoi(token);
    } else if (strncmp("NLH3", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      if (N_LAYERS < 5) {
	cerr << "Error: invalid option in .param file" << endl;
	cerr << "       NLH3 can only be used if NLAYER > 4" << endl;
	exit(10);
      }
      nll[3] = atoi(token);
    } else if (strncmp("NLH4", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      if (N_LAYERS < 6) {
	cerr << "Error: invalid option in .param file" << endl;
	cerr << "       NLH4 can only be used if NLAYER > 5" << endl;
	exit(10);
      }
      nll[4] = atoi(token);
    } else if (strncmp("NLH5", token, 4) == 0) {
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      if (N_LAYERS < 7) {
	cerr << "Error: invalid option in .param file" << endl;
	cerr << "       NLH5 can only be used if NLAYER > 6" << endl;
	exit(10);
      }
      nll[5] = atoi(token);
    } else if (strncmp("NFILTER", token, 5) == 0) {
      HAS_NFILTER = 1;
      token = strtok(NULL, delims);
      token = strtok(NULL, delims);
      N_FILTERS = atoi(token);
      nll[0] = N_FILTERS;
    }
  }
  nll[N_LAYERS-1] = 1;
  if (HAS_MINZ + HAS_MAXZ == 0) {
    cerr << "ERROR: param file missing required parameters" << endl;
    exit(1);
  }
  in.close();

  nlayer_size = Vec_INT(N_LAYERS);
  for(int i=0; i<N_LAYERS; ++i) {
    nlayer_size[i] = nll[i];
  }

  //cout << "N_LAYERS = " << N_LAYERS << ", N_FILTERS = " << N_FILTERS << endl;
  //cout << "min_z = " << min_z << ", max_z = " << max_z << endl;
  //for (int i=0; i<N_LAYERS; ++i) {
  //  cout << "Layer " << i << ": N_node = " << nlayer_size[i] << endl;
  //}

  delete [] line;
  delete [] delims;
}

int neu_net::initialize() {

  N_W = 0;
  for (int i = 0; i < N_LAYERS-1; ++i) {
    N_W += nlayer_size[i]*nlayer_size[i+1];
  }
  //cout << "N_W = " << N_W << endl;
  long seed = 100098;
  double ddtemp = 0.1;
  //-----------------------------------------------------------------
  // new version
  //
  int NN = nlayer_size.size();
  //find the maximum nlayer_size
  MAX_LSIZE = -200000;
  for (int i = 0; i < NN; ++i) {
    if (MAX_LSIZE < nlayer_size[i])
      MAX_LSIZE = nlayer_size[i];
  }
  //cout << "MAX_LSIZE = " << MAX_LSIZE << endl;
  nnet = Mat_DP(N_LAYERS, MAX_LSIZE);
  nbeta = Mat_DP(N_LAYERS, MAX_LSIZE);
  nweights = Mat3D_DP(N_LAYERS-1, MAX_LSIZE, MAX_LSIZE);
  //ndw = Mat3D_DP(N_LAYERS-1, MAX_LSIZE, MAX_LSIZE);
  ndedw = Mat3D_DP(N_LAYERS-1, MAX_LSIZE, MAX_LSIZE);
  for (int i = 0; i < N_LAYERS; ++i) {
    for (int j = 0; j < MAX_LSIZE; ++j) {
      nnet[i][j] = 0.0;
      nbeta[i][j] = 0.0;
    }
  }
  for (int i = 0; i < N_LAYERS-1; ++i) {
    for (int j = 0; j < MAX_LSIZE; ++j) {
      for (int k = 0; k < MAX_LSIZE; ++k) {
	nweights[i][j][k] = 0.0;
	ndedw[i][j][k] = 0.0;
	//ndw[i][j][k] = 0.0;
      }
    }
  }
  K = 1.0;
  return 0;
}

void neu_net::randomizeWeights() {
  timeval *t1 = new timeval();
  gettimeofday(t1, NULL);
  long seed = t1->tv_usec;
  double d = 0.0;
  for(int i = 0; i < 1000; ++i)
    d = ran2(&seed);
  for (int i = 0; i < N_LAYERS-1; ++i) {
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	nweights[i][j][k] = (2.0*ran2(&seed)-1.0) / (DP)nlayer_size[i+1];
      }
    }
  }
  delete t1;
}

int neu_net::clean() {

  return 0;
}
int neu_net::read_trainfile(char *filename) {
  
  if (filename == NULL) {
    cerr << "invalid file name encountered" << endl;
    return 1;
  }
  FILE *in = NULL;
  in = fopen(filename, "r");
  char line[1024];
  if (in == NULL) {
    cerr << "error opening the file: filename = " << filename << endl;
    return 2;
  }
  if (N_FILTERS < 1 || N_FILTERS > 10) {
    cerr << "invalid number of filters encountered in read_data... " << endl;
    return 3;
  }

  
  int NIN = 2 + N_FILTERS*2;
  double tempdouble;
  N_DATAPOINTS = 0;
  while(!feof(in)) {
    fgets(line,1024,in);
    N_DATAPOINTS++;
  }
  N_DATAPOINTS--;
  fclose(in);

  //fscanf(in, "%d", &N_DATAPOINTS);
  if (N_DATAPOINTS < 1 || N_DATAPOINTS > 100000000) {
    cerr << "invalid number of data points in the file..." << endl;
    return 4;
  }

  cout << "reading trainfile: N_DATAPOINT = " << N_DATAPOINTS << ", N_FILTER = " << N_FILTERS << endl;
  in = fopen(filename, "r");
  zdat = Vec_DP(N_DATAPOINTS);
  stype = Vec_DP(N_DATAPOINTS);
  zphot = Vec_DP(N_DATAPOINTS);
  sigma_zphot = Vec_DP(N_DATAPOINTS);
  zscaled = Vec_DP(N_DATAPOINTS);
  stype_phot = Vec_DP(N_DATAPOINTS);
  mags = Mat_DP(N_DATAPOINTS, N_FILTERS);
  mag_errs = Mat_DP(N_DATAPOINTS, N_FILTERS);


  for (int i = 0; i < N_DATAPOINTS; ++i) {
    //cout << "i = " << i << endl;
    fgets(line, 1024, in);
    char *token = strtok(line, " \n");
    zdat[i] = atof(token);
    token = strtok(NULL, " \n");
    stype[i] = atof(token);
    for (int j = 0; j < N_FILTERS; ++j) {
      token = strtok(NULL, " \n");
      mags[i][j] = normalize(atof(token));
    }
    for (int j = 0; j < N_FILTERS; ++j) {
      token = strtok(NULL, " \n");
      mag_errs[i][j] = atof(token);
      //mag_errs[i][j] = normalize(mag_errs[i][j]);
    }
  }
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    zscaled[i] = scale_z(zdat[i]);
  }
  //cout << "max_z = " << max_z << ", min_z = " << min_z << ", max_mag = " << max_mag << ", min_mag = " << min_mag << endl;
  
  return 0;
}

int neu_net::read_datafile(char *filename) {
  
  if (filename == NULL) {
    cerr << "invalid file name encountered" << endl;
    return 1;
  }
  FILE *in = NULL;
  in = fopen(filename, "r");
  if (in == NULL) {
    cerr << "error opening the file: filename = " << filename << endl;
    return 2;
  }
  if (N_FILTERS < 2 || N_FILTERS > 10) {
    cerr << "invalid number of filters encountered in read_data... " << endl;
    return 3;
  }

  int NIN = N_FILTERS*2;
  double tempdouble;
  N_DATAPOINTS = 0;
  while(!feof(in)) {
    for(int i=0; i<NIN; ++i) fscanf(in, "%lf", &tempdouble);
    N_DATAPOINTS++;
  }
  N_DATAPOINTS--;
  fclose(in);

  if (N_DATAPOINTS < 1 || N_DATAPOINTS > 100000000) {
    cerr << "invalid number of data points in the file..." << endl;
    return 4;
  }

  cout << "reading datafile: N_DATAPOINT = " << N_DATAPOINTS << ", N_FILTER = " << N_FILTERS << endl;
  in = fopen(filename, "r");
  zphot = Vec_DP(N_DATAPOINTS);
  sigma_zphot = Vec_DP(N_DATAPOINTS);
  stype_phot = Vec_DP(N_DATAPOINTS);
  mags = Mat_DP(N_DATAPOINTS, N_FILTERS);
  mag_errs = Mat_DP(N_DATAPOINTS, N_FILTERS);
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    for (int j = 0; j < N_FILTERS; ++j) {
      fscanf(in, "%lf", &mags[i][j]);
      mags[i][j] = normalize(mags[i][j]);
    }
    for (int j = 0; j < N_FILTERS; ++j) {
      fscanf(in, "%lf", &mag_errs[i][j]);
      //mag_errs[i][j] = mag_errs[i][j];
    }
  }
  //cout << "max_z = " << max_z << ", min_z = " << min_z << ", max_mag = " << max_mag << ", min_mag = " << min_mag << endl;
  
  return 0;
}


double neu_net::EE(Vec_I_DP &w) {
  double result = 0.0;
  double tempd;
  if (mags.nrows() != zdat.size()) {
    cerr << "size mismatch on mags.nrows" << endl;
    cerr  << "mags.nrows() = " << mags.nrows() << ", zdat.size() = " << zdat.size() << endl;
    return 0.0;
  }
  if (mags.ncols() != N_FILTERS) {
    return 0.0;
  }

  //--------------------------------------------------------------------------
  //first zero beta, net, and dedw
  for (int i = 0; i < N_LAYERS; ++i) {
    for (int j = 0; j < MAX_LSIZE; ++j) {
      nnet[i][j] = 0.0;
      nbeta[i][j] = 0.0;
    }
  }
  //--------------------------------------------------------------------------

  for (int id = 0; id < N_DATAPOINTS; ++id) {
    for (int i = 0; i < N_FILTERS; ++i) {
      //nnet[0][i] = sigmoid(mags[id][i]);
      nnet[0][i] = mags[id][i];
    }
    for (int k = 0; k < nlayer_size[1]; ++k) {
      double temp = 0.0;
      for (int j = 0; j < nlayer_size[0]; ++j) {
	temp += w[j*nlayer_size[1] + k] * nnet[0][j];
      }
      nnet[1][k] = sigmoid(temp);
    }
    int tempii = 0;
    for (int i = 1; i < N_LAYERS-1; ++i) {
      tempii += nlayer_size[i]*nlayer_size[i-1];
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	double temp = 0.0;
	for (int j = 0; j < nlayer_size[i]; ++j) {
	  temp += w[tempii + j*nlayer_size[i+1] + k] * nnet[i][j];
	}
	nnet[i+1][k] = sigmoid(temp);
      }
    }
    result += (0.5*(zscaled[id] - nnet[N_LAYERS-1][0])*
	       (zscaled[id] - nnet[N_LAYERS-1][0]));
    //cout << "result = " << result << endl;
  }  //loop over N_DATAPOINTS
  //calculate the weight regulator
  double Ew = 0.0;
  for (int k = 0; k < nlayer_size[1]; ++k) {
    double temp = 0.0;
    for (int j = 0; j < nlayer_size[0]; ++j) {
      Ew += w[j*nlayer_size[1] + k]*w[j*nlayer_size[1] + k];
    }
  }
  int tempii = 0;
  for (int i = 1; i < N_LAYERS-1; ++i) {
    tempii += nlayer_size[i]*nlayer_size[i-1];
    for (int k = 0; k < nlayer_size[i+1]; ++k) {
      double temp = 0.0;
      for (int j = 0; j < nlayer_size[i]; ++j) {
	  Ew += w[tempii + j*nlayer_size[i+1] + k]*w[tempii + j*nlayer_size[i+1] + k];
      }
    }
  }
  result += (Ew*wbeta);
  return result;
}

void neu_net::dE(Vec_I_DP &w, Vec_O_DP &dde) {
  double result = 0.0;
  int tempii;
  if (mags.nrows() != zdat.size()) {
    cerr << "size mismatch on mags.nrows" << endl;
    cerr  << "mags.nrows() = " << mags.nrows() << ", zdat.size() = " << zdat.size() << endl;
    return;
  }
  if (mags.ncols() != N_FILTERS) {
    return;
  }
  if (dde.size() != w.size()) {
    return;
  }
  //--------------------------------------------------------------------------
  //first zero beta, net, and dedw
  for (int i = 0; i < N_LAYERS; ++i) {
    for (int j = 0; j < MAX_LSIZE; ++j) {
      nnet[i][j] = 0.0;
      nbeta[i][j] = 0.0;
    }
  }
  for (int i = 0; i < N_LAYERS-1; ++i) {
    for (int j = 0; j < MAX_LSIZE; ++j) {
      for (int k = 0; k < MAX_LSIZE; ++k) {
	ndedw[i][j][k] = 0.0;
      }
    }
  }
  //--------------------------------------------------------------------------
  
  for (int id = 0; id < N_DATAPOINTS; ++id) {
    for (int i = 0; i < N_LAYERS; ++i) {
      for (int j = 0; j < MAX_LSIZE; ++j) {
	nnet[i][j] = 0.0;
	nbeta[i][j] = 0.0;
      }
    }
    for (int i = 0; i < N_FILTERS; ++i) {
      //nnet[0][i] = sigmoid(mags[id][i]);
      nnet[0][i] = mags[id][i];
    }
    for (int k = 0; k < nlayer_size[1]; ++k) {
      double temp = 0.0;
      for (int j = 0; j < nlayer_size[0]; ++j) {
	//if (id == 0) cout << j*nlayer_size[1] + k << endl;
	temp += w[j*nlayer_size[1] + k] * nnet[0][j];
      }
      nnet[1][k] = sigmoid(temp);
    }
    tempii = 0;
    for (int i = 1; i < N_LAYERS-1; ++i) {
      tempii += nlayer_size[i]*nlayer_size[i-1];
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	double temp = 0.0;
	for (int j = 0; j < nlayer_size[i]; ++j) {
	  temp += w[tempii + j*nlayer_size[i+1] + k] * nnet[i][j];
	}
 	nnet[i+1][k] = sigmoid(temp);
      }
    }
    //--------------------------------------------------------------------------
    //calculate the betas now
    nbeta[N_LAYERS-1][0] = (nnet[N_LAYERS-1][0] - zscaled[id]);
    //now the rest
    tempii = N_W;
    for (int i = N_LAYERS-2; i > 0; --i) { 
      //Note: here, the order of this loop matters.
      //Must loop from high i to low i, because for each beta, I need the betas of the layer above
      tempii -= nlayer_size[i]*nlayer_size[i+1];
      for (int j = 0; j < nlayer_size[i]; ++j) {
	for (int k = 0; k < nlayer_size[i+1]; ++k) {
	  //cout << "i = " << i << ", j = " << j << ", k = " << k << ", tempii + j*nlayer_size[i+1] + k = " << tempii + j*nlayer_size[i+1] + k << endl;
	  nbeta[i][j] += w[tempii + j*nlayer_size[i+1] + k]*sigmoid_prime(nnet[i+1][k])*nbeta[i+1][k];
	}
      }
    }
    //--------------------------------------------------------------------------
    
    //--------------------------------------------------------------------------
    //finally, calculate the derivatives
    for (int i = N_LAYERS-2; i >= 0; --i) {
      for (int j = 0; j < nlayer_size[i]; ++j) {
	for (int k = 0; k < nlayer_size[i+1]; ++k) {
	  ndedw[i][j][k] += nnet[i][j]*sigmoid_prime(nnet[i+1][k])*nbeta[i+1][k];
	}
      }
    }
    //--------------------------------------------------------------------------
    
    //make sure to initialize dde
    for (int i = 0; i < dde.size(); ++i)
      dde[i] = 0.0;
    //unrolled loop...
    //i = 0
    for (int j = 0; j < nlayer_size[0]; ++j) {
      for (int k = 0; k < nlayer_size[1]; ++k) {
	dde[j*nlayer_size[1] + k] = ndedw[0][j][k];
      }
    }
    //now the rest
    tempii = 0;
    for (int i = 1; i < N_LAYERS-1; ++i) {
      tempii += nlayer_size[i]*nlayer_size[i-1];
      for (int j = 0; j < nlayer_size[i]; ++j) {
	for (int k = 0; k < nlayer_size[i+1]; ++k) {
	  dde[tempii + j*nlayer_size[i+1] + k] = ndedw[i][j][k];
	}
      }
    }
  }
  //----------------------------------------------------------------------------
  //Do the regulators
  for (int j = 0; j < nlayer_size[0]; ++j) {
    for (int k = 0; k < nlayer_size[1]; ++k) {
      dde[j*nlayer_size[1] + k] += 2.0 * wbeta * w[j*nlayer_size[1] + k];
    }
  }
  tempii = 0;
  for (int i = 1; i < N_LAYERS-1; ++i) {
    tempii += nlayer_size[i]*nlayer_size[i-1];
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	dde[tempii + j*nlayer_size[i+1] + k] += 2.0 * wbeta * w[tempii + j*nlayer_size[i+1] + k];
      }
    }
  }
}
int neu_net::calc_zphot() {

  const double epsilon = 1.e-5;
  double tempd = 0.0;
  double result = 0.0;
  double *zp_tmp = new double[N_FILTERS];
  double *dzdm = new double[N_FILTERS];
  for (int id = 0; id < N_DATAPOINTS; ++id) {
    //cout << "id = " << id << endl;
    for (int i = 0; i < N_FILTERS; ++i) {
      //nnet[0][i] = sigmoid(mags[id][i]);
      nnet[0][i] = mags[id][i];
    }
    //printf("%lf %lf %lf %lf %lf\n", nnet[0][0], nnet[0][1], nnet[0][2], nnet[0][3], nnet[0][4]);
    for (int i = 1; i < N_LAYERS; ++i) {
      for (int j = 0; j < nlayer_size[i]; ++j) {
	double temp = 0.0;
	for (int k = 0; k < nlayer_size[i-1]; ++k) {
	  temp += nweights[i-1][k][j] * nnet[i-1][k];
	}
	nnet[i][j] = sigmoid(temp);
      }
    }
    zphot[id] = unscale_z(nnet[N_LAYERS-1][0]);
    //printf("%lf %lf %lf\n", zphot[id], zdat[id], zphot[id]-zdat[id]);
    stype_phot[id] = unscale_sed(nnet[N_LAYERS-1][1]);
    //now, calculate the sigma
    for(int i=0; i<N_FILTERS; ++i) {
      double z1 = 0.0;
      double z2 = 0.0;
      //plus epsilon first
      for(int j=0; j<N_FILTERS; ++j) nnet[0][j] = mags[id][j];
      nnet[0][i] = normalize(unnormalize(mags[id][i])*(1.0 + epsilon));
      for (int ii = 1; ii < N_LAYERS; ++ii) {
	for (int jj = 0; jj < nlayer_size[ii]; ++jj) {
	  double temp = 0.0;
	  for (int k = 0; k < nlayer_size[ii-1]; ++k) {
	    temp += nweights[ii-1][k][jj] * nnet[ii-1][k];
	  }
	  nnet[ii][jj] = sigmoid(temp);
	}
      }
      z1 = unscale_z(nnet[N_LAYERS-1][0]);
      //minus epsilon next
      for(int j=0; j<N_FILTERS; ++j) nnet[0][j] = mags[id][j];
      nnet[0][i] = normalize(unnormalize(mags[id][i])*(1.0 - epsilon));
      for (int ii = 1; ii < N_LAYERS; ++ii) {
	for (int jj = 0; jj < nlayer_size[ii]; ++jj) {
	  double temp = 0.0;
	  for (int k = 0; k < nlayer_size[ii-1]; ++k) {
	    temp += nweights[ii-1][k][jj] * nnet[ii-1][k];
	  }
	  nnet[ii][jj] = sigmoid(temp);
	}
      }
      z2 = unscale_z(nnet[N_LAYERS-1][0]);
      //dzdm = (z(m+epsilon) - z(m-epsilon)) / (2*epsilon)
      dzdm[i] = (z1 - z2) / (2.*epsilon*unnormalize(mags[id][i]));
    }
    sigma_zphot[id] = 0.0;
    for(int i=0; i<N_FILTERS; ++i) {
      sigma_zphot[id] += (dzdm[i]*dzdm[i]*mag_errs[id][i]*mag_errs[id][i]);
    }
    sigma_zphot[id] = sqrt(sigma_zphot[id]);
  }
  delete [] zp_tmp;
  delete [] dzdm;
  return 0;
}


int neu_net::calcZphot() {
  double result = 0.0;
  for (int id = 0; id < N_DATAPOINTS; ++id) {
    for (int i = 0; i < N_FILTERS; ++i) {
      nnet[0][i] = mags[id][i];
    }
    for (int i = 1; i < N_LAYERS; ++i) {
      for (int j = 0; j < nlayer_size[i]; ++j) {
	double temp = 0.0;
	for (int k = 0; k < nlayer_size[i-1]; ++k) {
	  temp += nweights[i-1][k][j] * nnet[i-1][k];
	}
	nnet[i][j] = sigmoid(temp);
      }
    }
    zphot[id] = unscale_z(nnet[N_LAYERS-1][0]);
  }
  return 0;
}

double neu_net::normalize(double mag) {
  return (4.0*((mag-min_mag)/(max_mag-min_mag) - 0.5));
}

double neu_net::unnormalize(double mag) {
  return ((max_mag-min_mag)*(0.25*mag + 0.5) + min_mag);
}

inline double neu_net::sigmoid(double d) {
  return (1.0 / (1.0 + exp(-1.0*K*d)));
}

inline double neu_net::sigmoid_prime(double d) {
  return (d * (1.0 - d) * K);
}
 
 double neu_net::inv_sigmoid(double d) {
   if (d > 0.9999999999999) {
     return 30.0;
   }
  return (log(d / (1.0 - d)) / K);
}

double neu_net::scale_z(double z) {
  return ((z-min_z)/(max_z-min_z));
  //return (z / (max_z*1.1));
  //return (2.0*sigmoid(z)-1.0);
  //return (sigmoid(z));
}

double neu_net::unscale_z(double z) {
  return (z*(max_z-min_z) + min_z);
  //return (2.0*sigmoid(z)-1.0);
  //return (inv_sigmoid(z));
}

double neu_net::scale_sed(double s) {
  return ((s-min_s)/(max_s - min_s));
  //return (2.0*sigmoid(z)-1.0);
  //return (sigmoid(z));
}

double neu_net::unscale_sed(double s) {
  return (s*(max_s-min_s) + min_s);
  //return (2.0*sigmoid(z)-1.0);
  //return (inv_sigmoid(z));
}

int neu_net::set_eta(double d) {
  eta = d;
  return 0;
}
int neu_net::set_alpha(double d) {
  alpha = d;
  return 0;
}

int neu_net::set_mbias(double d) {
  mbias = d;
  return 0;
}

int neu_net::set_wbeta(double d) {
  wbeta = d;
  return 0;
}

int neu_net::set_K(double d) {
  K = d;
  return 0;
}

int neu_net::new_minimize() {

  double factor = 1.0;

  /*
  for (int i = N_LAYERS-2; i >= 0; --i) {
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	ndw[i][j][k] = -1.0*ndedw[i][j][k]*eta + alpha*ndw[i][j][k];
	nweights[i][j][k] += ndw[i][j][k];
      }
    }
  }
  */
  return 0;
}
int neu_net::minimize() {
  
  //calculate de/dw

  //do the output layer first (this is backward propagation method)
  for (int i = 0; i < N_LAYERS-1; ++i) {
    double *wptr = weights[i];
    double *dedwptr = dedw[i];
    double *dwptr = dw[i];
    for (int j = 0; j < layer_size[i]*layer_size[i+1]; ++j) {
      //cout << "i = " << i << ", j = " << j << ", wptr = " << *wptr << ", dedwptr = " << *dedwptr << ", dwptr = " << *dwptr  << endl;
      *dwptr = -1.0*(*dedwptr)*eta + alpha*(*dwptr);
      *wptr += *dwptr;
      ++wptr;
      ++dwptr;
      ++dedwptr;
    }
  }
  return 0;
}

int neu_net::minimize(Vec_IO_DP &w, Vec_I_DP &dde) {
  for (int i = 0; i < N_W; ++i) {
    w[i] += -1.0 * eta * dde[i];
  }
  return 0;
}

int neu_net::maximize(Vec_IO_DP &w, Vec_I_DP &dde) {
  for (int i = 0; i < N_W; ++i) {
    w[i] += 1.0 * eta * dde[i];
  }
  return 0;
}


int neu_net::print_z() {

  double zave = 0.0;
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    cout << "mag1[" << i << "] = " << mags[i][0] << ", zdat[" << i << "] = " << zdat[i] << ", zphot[" << i << "] = " << zphot[i] << endl;
    zave += zdat[i];
  }
  zave /= N_DATAPOINTS;
  cout << "zave = " << zave << endl;
  return 0;
}

int neu_net::write_z(char *filename) {
  FILE *out = NULL;
  out = fopen(filename, "w");
  if (out == NULL) {
    cerr << "error opening " << filename << endl;
    return 1;
  }
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    fprintf(out, "%lf %lf\n", zdat[i], zphot[i]);
  }
  fclose(out);
  return 0;
}

int neu_net::write_sed(char *filename) {
  FILE *out = NULL;
  out = fopen(filename, "w");
  if (out == NULL) {
    cerr << "error opening " << filename << endl;
    return 1;
  }
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    fprintf(out, "%lf %lf\n", stype[i], stype_phot[i]);
  }
  fclose(out);
  return 0;
}

double neu_net::z_score() {
  double result = 0.0;
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    result += 0.5*(zdat[i]-zphot[i])*(zdat[i]-zphot[i]);
  }
  return result;
}

double neu_net::z_scatter() {
  double result = 0.0;
  double zave = 0.0;
  zave /= N_DATAPOINTS;
  const double epsilon = 1.e-5;
  double tempd = 0.0;
  for (int id = 0; id < N_DATAPOINTS; ++id) {
    for (int i = 0; i < N_FILTERS; ++i) {
      nnet[0][i] = mags[id][i];
    }
    for (int i = 1; i < N_LAYERS; ++i) {
      for (int j = 0; j < nlayer_size[i]; ++j) {
	double temp = 0.0;
	for (int k = 0; k < nlayer_size[i-1]; ++k) {
	  temp += nweights[i-1][k][j] * nnet[i-1][k];
	}
	nnet[i][j] = sigmoid(temp);
      }
    }
    zphot[id] = unscale_z(nnet[N_LAYERS-1][0]);
    stype_phot[id] = unscale_sed(nnet[N_LAYERS-1][1]);
  }
  for (int i = 0; i < N_DATAPOINTS; ++i) {
    result += (zdat[i]-zphot[i])*(zdat[i]-zphot[i]);
  }
  result /= (double)N_DATAPOINTS;
  result = sqrt(result);
  return result;
}

int neu_net::rw(char *filename) {
  
  if (filename == NULL) {
    cerr << "error in read_weights: filename is null" << endl;
    return 1;
  }
  cout << "reading weight file: " << filename << "  ...." << flush;
  FILE *in = NULL;
  in = fopen(filename, "r");
  if (in == NULL) {
    cerr << "error opening the weights file: " << filename << endl;
    return 1;
  }
  int N_L;
  fscanf(in, "%d", &N_L);
  Vec_INT NLS(N_L);
  for (int i = 0; i < N_L; ++i) {
    fscanf(in, "%d", &NLS[i]);
  }
  clean();
  //initialize();
  
  for (int i = 0; i < N_LAYERS-1; ++i) {
    for (int j = 0; j < NLS[i]; ++j) {
      for (int k = 0; k < NLS[i+1]; ++k) {
	fscanf(in, "%lf", &nweights[i][j][k]);
      }
    }
  }
  fclose(in);
  cout << " done.  N_LAYERS = " << N_LAYERS << ", N_FILTERS = " << N_FILTERS << endl;
  return 0;

}

int neu_net::read_weights(char *filename) {
  
  if (filename == NULL) {
    cerr << "error in read_weights: filename is null" << endl;
    return 1;
  }
  cout << "reading weight file: " << filename << "  ...." << flush;
  FILE *in = NULL;
  in = fopen(filename, "r");
  if (in == NULL) {
    cerr << "error opening the weights file: " << filename << endl;
    return 1;
  }
  int N_L;
  fscanf(in, "%d", &N_L);
  Vec_INT NLS(N_L);
  for (int i = 0; i < N_L; ++i) {
    fscanf(in, "%d", &NLS[i]);
  }
  if (N_FILTERS != NLS[0]) {
    cerr << "NFILTERS does not match in read_weights()" << endl;
    return 1;
  }
  if (nlayer_size[1] != NLS[1]) {
    cerr << "nlayer_size[1] does not match in read_weights()" << endl;
    return 2;
  }
  if (nlayer_size[2] != NLS[2]) {
    cerr << "nlayer_size[2] does not match in read_weights()" << endl;
    return 3;
  }
  clean();
  //initialize();
  
  for (int i = 0; i < N_LAYERS-1; ++i) {
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	fscanf(in, "%lf", &nweights[i][j][k]);
      }
    }
  }
  fclose(in);
  cout << " done.  N_LAYERS = " << N_LAYERS << ", N_FILTERS = " << N_FILTERS << endl;
  return 0;

}

int neu_net::get_weights(Vec_O_DP &w) {
  if (w.size() != N_W) {
    cerr << "size mimatch in get_weights" << endl;
    return 1;
  }
  for (int j = 0; j < nlayer_size[0]; ++j) {
    for (int k = 0; k < nlayer_size[1]; ++k) {
      w[j*nlayer_size[1] + k] = nweights[0][j][k];
    }
  }
  int tempii = 0;
  for (int i = 1; i < N_LAYERS-1; ++i) {
    tempii += nlayer_size[i]*nlayer_size[i-1];
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	w[tempii + j*nlayer_size[i+1] + k] = nweights[i][j][k];
      }
    }
  }  
}
int neu_net::set_weights(Vec_I_DP &w) {
  if (w.size() != N_W) {
    cerr << "size mimatch in set_weights" << endl;
    return 1;
  }
  for (int j = 0; j < nlayer_size[0]; ++j) {
    for (int k = 0; k < nlayer_size[1]; ++k) {
      nweights[0][j][k] = w[j*nlayer_size[1] + k];
    }
  }
  int tempii = 0;
  for (int i = 1; i < N_LAYERS-1; ++i) {
    tempii += nlayer_size[i]*nlayer_size[i-1];
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	nweights[i][j][k] = w[tempii + j*nlayer_size[i+1] + k];
      }
    }
  }
  return 0;
}

int neu_net::write_weights(char *filename) {

  FILE *out = NULL;
  out = fopen(filename, "w");
  if (out == NULL) {
    cerr << "error opening the weights file: " << filename << endl;
    return 1;
  }
  
  fprintf(out, "%d\n", N_LAYERS);
  for (int i = 0; i < N_LAYERS; ++i) {
    fprintf(out, "%d ", nlayer_size[i]);
  }
  fprintf(out, "\n");
  for (int i = 0; i < N_LAYERS-1; ++i) {
    for (int j = 0; j < nlayer_size[i]; ++j) {
      for (int k = 0; k < nlayer_size[i+1]; ++k) {
	fprintf(out, "%22.16E ", nweights[i][j][k]);
      }
    }
    fprintf(out, "\n");
  }
  fclose(out);
  return 0;

}

//access functions
int neu_net::get_NL() {
  return N_LAYERS;
}

int neu_net::get_NDATA() {
  return N_DATAPOINTS;
}

int neu_net::get_NFILTERS() {
  return N_FILTERS;
}

Vec_DP neu_net::get_zphot() {
  return zphot;
}

Vec_DP neu_net::get_sigma_zphot() {
  return sigma_zphot;
}

Vec_DP neu_net::get_zspec() {
  return zdat;
}

Vec_DP neu_net::get_sphot() {
  return stype_phot;
}

Vec_DP neu_net::get_stype() {
  return stype;
}

Mat_DP neu_net::get_mags() {
  Mat_DP result = mags;
  for(int i = 0; i < result.nrows(); ++i) {
    for(int j = 0; j < result.ncols(); ++j) {
      result[i][j] = unnormalize(result[i][j]);
    }
  }
  return result;
}

Mat_DP neu_net::get_magerrs() {
  Mat_DP result = mag_errs;
  for(int i = 0; i < result.nrows(); ++i) {
    for(int j = 0; j < result.ncols(); ++j) {
      result[i][j] = result[i][j];
    }
  }
  return result;
}

Vec_INT neu_net:: get_nlayersize() {
  return nlayer_size;
}

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

double ran2(long *idum)
{
  int j;
  long k;
  static long idum2=123456789;
  static long iy=0;
  static long iv[NTAB];
  double temp;

  if (*idum <= 0) {
    if (-(*idum) < 1) *idum=1;
    else *idum = -(*idum);
    idum2=(*idum);
    for (j=NTAB+7;j>=0;j--) {
      k=(*idum)/IQ1;
      *idum=IA1*(*idum-k*IQ1)-k*IR1;
      if (*idum < 0) *idum += IM1;
      if (j < NTAB) iv[j] = *idum;
    }
    iy=iv[0];
  }
  k=(*idum)/IQ1;
  *idum=IA1*(*idum-k*IQ1)-k*IR1;
  if (*idum < 0) *idum += IM1;
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2;
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV;
  iy=iv[j]-idum2;
  iv[j] = *idum;
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX;
  else return temp;
}
#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX
