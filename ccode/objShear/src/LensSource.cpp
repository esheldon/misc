#include "LensSource.h"
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

#include "gcirc.h"

using namespace std;

LensSource::LensSource() {} // default constructor does nothing
LensSource::LensSource(string pParFile) { // constructor

    mDebug=0;
    mDebug2=0;
    ReadPar(pParFile);

    ReadScatOld(mPars.source_file);
    ReadLcat(mPars.lens_file);
    ReadRev(mPars.rev_file);

    if (mPars.sigmacrit_style == 1)
        CalcAeta();
    else if (mPars.sigmacrit_style == 3)
        ReadScinv2d(mPars.scinv_file); 

    // Initialize the htm index
    mHTM = new htmInterface( (size_t) mPars.depth );
    mSpatialIndex = &mHTM->index();

}

/*
 *
 * Methods for reading files
 * 
 */ 

int32 LensSource::GetIdlstructNrows(ifstream& f)
{
    short i;
    char c;
    char junk[9];
    char nrows_string[11];

    f.read(junk, 9);
    f.read(nrows_string, 11);
    return( atoi(nrows_string) );

}



void LensSource::ReadPar(string file)
{

    // For holding the first column: the keyword for each parameter.
    string keyword;

    cout << endl <<
        "Reading parameters from file " << file << endl;

    ifstream f(file.c_str(), ios::in);
    if (!f) {
        cout << "Could not open par file: " << file << endl;
        exit(45);
    }

    f >> keyword >> mPars.lens_file;
    f >> keyword >> mPars.source_file;
    f >> keyword >> mPars.rev_file;
    f >> keyword >> mPars.scinv_file;
    f >> keyword >> mPars.output_file;
    f >> keyword >> mPars.pair_file;

    f >> keyword >> mPars.pixel_lensing;
    f >> keyword >> mPars.dopairs;

    f >> keyword >> mPars.h;
    f >> keyword >> mPars.omega_m;

    f >> keyword >> mPars.sigmacrit_style;
    f >> keyword >> mPars.shape_correction_style;


    f >> keyword >> mPars.logbin;
    f >> keyword >> mPars.nbin;

    f >> keyword >> mPars.binsize;

    f >> keyword >> mPars.rmin;
    f >> keyword >> mPars.rmax;

    f >> keyword >> mPars.comoving;
    f >> keyword >> mPars.depth;
    f >> keyword >> mPars.zbuffer;
    f.close();

    // Print out all the values
    cout << " lens_file = " << mPars.lens_file << endl;
    cout << " source_file = " << mPars.source_file << endl;
    cout << " rev_file = " << mPars.rev_file << endl;
    cout << " scinv_file = " << mPars.scinv_file << endl;
    cout << " output_file = " << mPars.output_file << endl;
    cout << " pairs_file = " << mPars.pair_file << endl;
    cout << " pixel_lensing? = " << mPars.pixel_lensing << endl;
    cout << " dopairs? = " << mPars.dopairs << endl;

    cout << " h = " << mPars.h << endl;
    cout << " omega_m = " << mPars.omega_m << endl;

    cout << " sigmacrit_style = " << mPars.sigmacrit_style << endl;
    cout << " shape_correction_style = " <<
        mPars.shape_correction_style << endl;

    cout << " logbin = " << mPars.logbin << endl;
    cout << " nbin = " << mPars.nbin << endl;

    cout << " binsize = " << mPars.binsize << endl;

    cout << " rmin = " << mPars.rmin << endl;
    cout << " rmax = " << mPars.rmax << endl;

    cout << " comoving = " << mPars.comoving << endl;
    cout << " depth = " << mPars.depth << endl;
    cout << " zbuffer = " << mPars.zbuffer << endl;

    // What type of binning?
    if (mPars.logbin) {
        printf("\nBinning in log\n");
        mPars.logRmin = log10(mPars.rmin);
        mPars.logRmax = log10(mPars.rmax);

        mPars.logBinsize =
            ( mPars.logRmax - mPars.logRmin )/mPars.nbin;

        cout << " logRmin = " << mPars.logRmin << endl;
        cout << " logRmax = " << mPars.logRmax << endl;
        cout << " logBinsize = " << mPars.logBinsize << endl;
    }


}


void LensSource::ReadScatOld(string file)
{

    ifstream f;
    char c;
    int32 nlines, row;

    f.open(file.c_str());
    if (!f) {
        cout << "Cannot open source file " << file << endl;
        exit(45);
    }

    // Read the number of rows
    mPars.nsource = GetIdlstructNrows(f);

    cout << endl 
        <<"Reading "<< mPars.nsource<<" sources from file "<<file<<endl;

    // That leaves us sitting on the first newline. 
    // There are SOURCE_HEADER_LINES header lines in total */

    nlines = 0;
    cout << "Skipping "<<SOURCE_HEADER_LINES<<" header lines"<<endl;
    while (nlines < SOURCE_HEADER_LINES) {
        c = f.get();
        if (c == '\n') nlines++;
    }

    cout<<"Allocating memory"<<endl;
    mSources.resize(mPars.nsource);

    cout<<"Reading...."<<endl;
    for (row=0; row<mPars.nsource; row++) {
        f.read((char *)&mSources[row].stripe,sizeof(int16));
        
        f.read((char *)&mSources[row].e1,sizeof(float));
        f.read((char *)&mSources[row].e2,sizeof(float));

        f.read((char *)&mSources[row].e1e1err,sizeof(float));
        f.read((char *)&mSources[row].e1e2err,sizeof(float));
        f.read((char *)&mSources[row].e2e2err,sizeof(float));

        f.read((char *)&mSources[row].clambda,sizeof(double));
        f.read((char *)&mSources[row].ceta,sizeof(double));

        f.read((char *)&mSources[row].z,sizeof(float));
        f.read((char *)&mSources[row].zerr,sizeof(float));

        f.read((char *)&mSources[row].htm_index,sizeof(int32));
    }
}




void LensSource::ReadScatPixel(string file)
{

    ifstream f;
    char c;
    int32 nlines, row;

    f.open(file.c_str());
    if (!f) {
        cout << "Cannot open pixel source file " << file << endl;
        exit(45);
    }


    f.read((char *)&mPars.nsource, sizeof(int32));


    cout << endl
       <<"Reading "<< mPars.nsource 
       <<" source pixels from file "<<file<<endl;
 
    mPixelSources.resize(mPars.nsource);

    for (int32 row=0; row < mPars.nsource; row++)
    {

        f.read((char *)&mPixelSources[row].htm_index, sizeof(int32));

        f.read((char *)&mPixelSources[row].nobj, sizeof(int16));

        f.read((char *)&mPixelSources[row].e1, sizeof(float32));
        f.read((char *)&mPixelSources[row].e2, sizeof(float32));
        f.read((char *)&mPixelSources[row].e1e1err, sizeof(float32));
        f.read((char *)&mPixelSources[row].e1e2err, sizeof(float32));
        f.read((char *)&mPixelSources[row].e2e2err, sizeof(float32));

        f.read((char *)&mPixelSources[row].aeta1, sizeof(float32));
        f.read((char *)&mPixelSources[row].aeta2, sizeof(float32));

        f.read((char *)&mPixelSources[row].clambda, sizeof(float64));
        f.read((char *)&mPixelSources[row].ceta, sizeof(float64));
    }


}


void LensSource::ReadRev(string file)
{

    ifstream f;

    f.open(file.c_str());
    if (!f) {
        cout << "Could not open reverse indices file " << file << endl;
        exit(45);
    }

    f.read((char *)&mPars.nrev, sizeof(int32));
    f.read((char *)&mPars.min_htm_index, sizeof(int32));
    f.read((char *)&mPars.max_htm_index, sizeof(int32));

    cout << endl
        <<"Reading "<<mPars.nrev<<" rev data from file "<<file<< endl;

    mRev.resize(mPars.nrev);
    f.read((char *)&mRev[0], mPars.nrev*sizeof(int32));

    f.close();

}



/* Reading man inverse critical density file */
void LensSource::ReadScinv2d(string file)
{

    FILE* fptr;
    size_t nread;

    cout <<"Reading scinv file: "<<file<<endl;
    if (! (fptr = fopen(file.c_str(), "r")) ) {
      cout << "Cannot open scinv file " << file << endl;
      exit(45);
    }

    nread=fread( &mScinv.npoints, sizeof(int32), 1, fptr);
    nread=fread( &mScinv.nzl, sizeof(int32), 1, fptr);
    nread=fread( &mScinv.nzs, sizeof(int32), 1, fptr);

    int32 nzl=mScinv.nzl;
    int32 nzs=mScinv.nzs;

    // Now that we know the sizes, allocate the arrays->
    mScinv.zl.resize(nzl);
    mScinv.zli.resize(nzl);
    mScinv.zs.resize(nzs);
    mScinv.zsi.resize(nzs);

    mScinv.scinv.resize(nzs*nzl);

    nread=fread( &mScinv.zlStep, sizeof(float32), 1, fptr);
    nread=fread( &mScinv.zlMin, sizeof(float32), 1, fptr);
    nread=fread( &mScinv.zlMax, sizeof(float32), 1, fptr);
    nread=fread( &mScinv.zl[0], sizeof(float32), nzl, fptr);
    nread=fread( &mScinv.zli[0], sizeof(float32), nzl, fptr);

    nread=fread( &mScinv.zsStep, sizeof(float32), 1, fptr);
    nread=fread( &mScinv.zsMin, sizeof(float32), 1, fptr);
    nread=fread( &mScinv.zsMax, sizeof(float32), 1, fptr);
    nread=fread( &mScinv.zs[0], sizeof(float32), nzs, fptr);
    nread=fread( &mScinv.zsi[0], sizeof(float32), nzs, fptr);

    for (int32 i=0; i<nzl; i++)
        for (int32 j=0; j<nzs; j++)
        {
            //nread=fread( &scinv[i*mScinv.nzl + j], sizeof(float64), 1, fptr);
            nread=fread( &mScinv.scinv[i*mScinv.nzl + j], 
                         sizeof(float64), 1, fptr);
        }

    fclose(fptr);

}





void LensSource::ReadLcat(string file)
{

    ifstream f;
    char c;
    int32 nlines, row;

    f.open(file.c_str());
    if (!f) {
        cout << "Cannot open lens file " << file << endl;
        exit(45);
    }

    // Read the number of rows
    mPars.nlens = GetIdlstructNrows(f);
    cout << endl
        <<"Reading "<<mPars.nlens<<" lenses from file "<<file<<endl;

    // That leaves us sitting on the first newline. 
    // There are LENS_HEADER_LINES header lines in total */

    nlines = 0;
    cout << "Skipping "<<LENS_HEADER_LINES<<" header lines"<<endl;
    while (nlines < LENS_HEADER_LINES) {
        c = f.get();
        if (c == '\n') nlines++;
    }

    cout<<"Allocating memory"<<endl;
    mLenses.resize(mPars.nlens);

    cout<<"Reading...."<<endl;
    for (row=0; row< mPars.nlens; row++) {
        f.read( (char *)&mLenses[row].zindex, sizeof(int32));

        f.read( (char *)&mLenses[row].ra, sizeof(double));
        f.read( (char *)&mLenses[row].dec, sizeof(double));
        f.read( (char *)&mLenses[row].clambda, sizeof(double));
        f.read( (char *)&mLenses[row].ceta, sizeof(double));

        f.read( (char *)&mLenses[row].z, sizeof(float));
        f.read( (char *)&mLenses[row].angmax, sizeof(float));
        f.read( (char *)&mLenses[row].DL, sizeof(float));

        f.read( (char *)&mLenses[row].mean_scinv, sizeof(float));

        f.read( (char *)&mLenses[row].pixelMaskFlags, sizeof(int16));
    }

}

/*
 *
 * Calculate the aetas for distances
 *
 */

void LensSource::CalcAeta()
{

  int32 i;
  float z_zero = 0.0;

  cout << endl << "Calculating aeta" << endl;

  mAetaZero = aeta(z_zero, mPars.omega_m);

  mScinvFactor = 0.001/1.663e3;

  cout<<"  Lenses...";fflush(stdout);
  mAetaRelLens.resize(mPars.nlens);

  float aeta0 = aeta(0.0, mPars.omega_m);

  for (i=0;i<mPars.nlens;i++)
    mAetaRelLens[i] = aeta0 - aeta(mLenses[i].z, mPars.omega_m);

  if (! mPars.pixel_lensing) {
      cout<<"  Sources...";fflush(stdout);
      mAetaRelSource.resize(mPars.nsource);
      for (i=0;i<mPars.nsource;i++)
          mAetaRelSource[i] = aeta0 - aeta(mSources[i].z, mPars.omega_m);
  }
  cout<<endl;

}




/*
 * Does what we read make sense?
 */


void LensSource::TestLcat()
{
  int32 nlens = mPars.nlens;
  int32 icheck;

  icheck = nlens-1;

  cout<<"Checking Lcat"<<endl;
  printf("%d %lf %lf %lf %lf %f %f %f %g %d\n", 
	 mLenses[0].zindex,
	 mLenses[0].ra,
	 mLenses[0].dec,
	 mLenses[0].clambda,
	 mLenses[0].ceta,
	 mLenses[0].z,
	 mLenses[0].angmax,
	 mLenses[0].DL,
	 mLenses[0].mean_scinv, 
	 mLenses[0].pixelMaskFlags);	 

  printf("%d %lf %lf %lf %lf %f %f %f %g %d\n", 
	 mLenses[icheck].zindex,
	 mLenses[icheck].ra,
	 mLenses[icheck].dec,
	 mLenses[icheck].clambda,
	 mLenses[icheck].ceta,
	 mLenses[icheck].z,
	 mLenses[icheck].angmax,
	 mLenses[icheck].DL,
	 mLenses[icheck].mean_scinv, 
	 mLenses[icheck].pixelMaskFlags);	 
}



void LensSource::TestScat()
{

  cout<<"Checking Scat"<<endl;
  int32 nsource = mPars.nsource;
  printf("%d %f %f %f %f %f %lf %lf %f %f %d\n", 
	 mSources[0].stripe, 
	 mSources[0].e1, 
	 mSources[0].e2, 
	 mSources[0].e1e1err, 
	 mSources[0].e1e2err, 
	 mSources[0].e2e2err, 
	 mSources[0].clambda, 
	 mSources[0].ceta, 
	 mSources[0].z, 
	 mSources[0].zerr, 
	 mSources[0].htm_index);


  printf("%d %f %f %f %f %f %lf %lf %f %f %d\n", 
	 mSources[nsource-1].stripe, 
	 mSources[nsource-1].e1, 
	 mSources[nsource-1].e2, 
	 mSources[nsource-1].e1e1err, 
	 mSources[nsource-1].e1e2err, 
	 mSources[nsource-1].e2e2err, 
	 mSources[nsource-1].clambda, 
	 mSources[nsource-1].ceta, 
	 mSources[nsource-1].z, 
	 mSources[nsource-1].zerr, 
	 mSources[nsource-1].htm_index);

}
void LensSource::TestScatPixel()
{

  int32 nsource = mPars.nsource;
  printf("%d %d   %f %f %f %f %f %f %f   %lf %lf\n", 
	 mPixelSources[nsource-1].htm_index,
	 mPixelSources[nsource-1].nobj,

	 mPixelSources[nsource-1].e1, 
	 mPixelSources[nsource-1].e2, 
	 mPixelSources[nsource-1].e1e1err, 
	 mPixelSources[nsource-1].e1e2err, 
	 mPixelSources[nsource-1].e2e2err, 

	 mPixelSources[nsource-1].aeta1, 
	 mPixelSources[nsource-1].aeta2, 

	 mPixelSources[nsource-1].clambda, 
	 mPixelSources[nsource-1].ceta);

}


void LensSource::TestRev()
{


    cout<<"Checking reverse indices"<<endl;
    cout<<"rev[0] = "<<mRev[0]<<"  rev["<<mRev.size()-1<<"] = "<<mRev[mRev.size()-1]<<endl;

    return;

}


/*
 * Convenience Functions
 */

int32 LensSource::GetNsource()
{
  return mPars.nsource;
}

int32 LensSource::GetNlens()
{
  return mPars.nlens;
}

float32 LensSource::GetScinvFactor()
{
  return mScinvFactor;
}
float32 LensSource::GetAetaZero()
{
  return mAetaZero;
}




/*
 *
 * Lens outputs
 *
 *
 */

void LensSource::MakeLensout()
{

    int32 nbin = mPars.nbin;
    mLensout.nbin = nbin;

    mLensout.npair.resize(nbin);

    mLensout.rmax_act.resize(nbin);
    mLensout.rmin_act.resize(nbin);

    mLensout.rsum.resize(nbin);

    mLensout.sigma.resize(nbin);
    mLensout.sigmaerr.resize(nbin);
    mLensout.orthosig.resize(nbin);
    mLensout.orthosigerr.resize(nbin);

    mLensout.sigerrsum.resize(nbin);
    mLensout.orthosigerrsum.resize(nbin);

    mLensout.wsum.resize(nbin);
    mLensout.owsum.resize(nbin);
    mLensout.wscritinvsum.resize(nbin);


    ResetLensout();

}
void LensSource::ResetLensout()
{

  mLensout.tot_pairs = 0;
  mLensout.weight = 0.0;
  mLensout.sshsum = 0.0;
  mLensout.wsum_ssh = 0.0;
  mLensout.ie = 9999.0;

  for (int32 i=0; i<mLensout.nbin; i++)
    {
      mLensout.npair[i] = 0;

      mLensout.rmax_act[i] = 0.0;
      mLensout.rmin_act[i] = 0.0;

      mLensout.rsum[i] = 0.0;

      mLensout.sigma[i] = 0.0;
      mLensout.sigmaerr[i] = 0.0;
      mLensout.orthosig[i] = 0.0;
      mLensout.orthosigerr[i] = 0.0;

      mLensout.sigerrsum[i] = 0.0;
      mLensout.orthosigerrsum[i] = 0.0;

      mLensout.wsum[i] = 0.0;
      mLensout.owsum[i] = 0.0;
      mLensout.wscritinvsum[i] = 0.0;
    }

}

void LensSource::WriteHeader()
{

    int32 nbin;
    nbin = mPars.nbin;

    // Required header keywords
    mOfstream<<"NROWS  = "<<std::right<<setw(15)<<mPars.nlens<<endl;
    mOfstream<<"FORMAT = BINARY"<<endl;
    mOfstream<<"BYTE_ORDER = NATIVE_LITTLE_ENDIAN"<<endl;


    mOfstream<<"IDLSTRUCT_VERSION = 0.9"<<endl;

    // optional keywords

    mOfstream<<"nlens = "<<mPars.nlens<<" # Number of lenses"<<endl;

    mOfstream<<"h = "<<mPars.h<<"   # Hubble parameter/100 km/s"<<endl;
    mOfstream<<"omega_m = "<<mPars.omega_m<<"  # omega_m, flat assumed"<<endl;

    mOfstream<<"sigmacrit_style = "<<mPars.sigmacrit_style<<" # How sigmacrit was calculated"<<endl;
    mOfstream<<"shape_correction_style = "<<mPars.shape_correction_style<<"  # How shape correction was performed"<<endl;

    mOfstream<<"logbin = "<<mPars.logbin<<"     # logarithmic binning?"<<endl;
    mOfstream<<"nbin =  "<<mPars.nbin<<"      # Number of bins"<<endl;
    mOfstream<<"binsize = "<<mPars.binsize<<"    # binsize if not log"<<endl;

    mOfstream<<"rmin = "<<mPars.rmin<<"       # mininum radius (kpc)"<<endl;
    mOfstream<<"rmax = "<<mPars.rmax<<"       # maximum radius (kpc)"<<endl;

    mOfstream<<"comoving = "<<mPars.comoving<<"   # comoving radii?"<<endl;

    mOfstream<<"htm_depth = "<<mPars.depth<<"  # depth of HTM search tree"<<endl;


    // field descriptions

    mOfstream<<"zindex 0L"<<endl;

    mOfstream<<"tot_pairs 0L"<<endl;
    mOfstream<<"npair lonarr("<<nbin<<")"<<endl;

    mOfstream<<"rmax_act fltarr("<<nbin<<")"<<endl;
    mOfstream<<"rmin_act fltarr("<<nbin<<")"<<endl;

    mOfstream<<"rsum fltarr("<<nbin<<")"<<endl;

    mOfstream<<"sigma fltarr("<<nbin<<")"<<endl;
    mOfstream<<"sigmaerr fltarr("<<nbin<<")"<<endl;
    mOfstream<<"orthosig fltarr("<<nbin<<")"<<endl;
    mOfstream<<"orthosigerr fltarr("<<nbin<<")"<<endl;

    mOfstream<<"sigerrsum fltarr("<<nbin<<")"<<endl;
    mOfstream<<"orthosigerrsum fltarr("<<nbin<<")"<<endl;

    mOfstream<<"weight 0.0"<<endl;

    mOfstream<<"wsum fltarr("<<nbin<<")"<<endl;
    mOfstream<<"owsum fltarr("<<nbin<<")"<<endl;
    mOfstream<<"wscritinvsum fltarr("<<nbin<<")"<<endl;

    mOfstream<<"sshsum 0.0"<<endl;
    mOfstream<<"wsum_ssh 0.0"<<endl;
    mOfstream<<"ie 0.0"<<endl;

    mOfstream<<"END"<<endl;
    mOfstream<<endl;

}

 
void LensSource::WriteLensout()
{

  int32 nbin = mLensout.nbin;

  mOfstream.write((char *)&mLensout.zindex, sizeof(int32));

  mOfstream.write((char *)&mLensout.tot_pairs, sizeof(int32));
  mOfstream.write((char *)&mLensout.npair[0],  sizeof(int32)*nbin);

  mOfstream.write((char *)&mLensout.rmax_act[0], sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.rmin_act[0], sizeof(float)*nbin);

  mOfstream.write((char *)&mLensout.rsum[0], sizeof(float)*nbin);

  mOfstream.write((char *)&mLensout.sigma[0],       sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.sigmaerr[0],    sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.orthosig[0],    sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.orthosigerr[0], sizeof(float)*nbin);

  mOfstream.write((char *)&mLensout.sigerrsum[0],      sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.orthosigerrsum[0], sizeof(float)*nbin);

  mOfstream.write((char *)&mLensout.weight, sizeof(float));

  mOfstream.write((char *)&mLensout.wsum[0],  sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.owsum[0], sizeof(float)*nbin);
  mOfstream.write((char *)&mLensout.wscritinvsum[0],  sizeof(float)*nbin);

  mOfstream.write((char *)&mLensout.sshsum,   sizeof(float));
  mOfstream.write((char *)&mLensout.wsum_ssh, sizeof(float));
  mOfstream.write((char *)&mLensout.ie,       sizeof(float));

}



void LensSource::WritePairHeader()
{
    int64 zero=0;
    mPairOfstream.write((char *)&zero, sizeof(int64));
}

void LensSource::WritePairs(
        int32 rLensIndex,
        int32 rSourceIndex,
        float32 rRkpc,
        float32 rWeight)
{
    mPairOfstream.write((char *)&rLensIndex, sizeof(int32));
    mPairOfstream.write((char *)&rSourceIndex, sizeof(int32));
    mPairOfstream.write((char *)&rRkpc, sizeof(float32));
    mPairOfstream.write((char *)&rWeight, sizeof(float32));
}

void LensSource::WriteTotPairs()
{
    mPairOfstream.seekp(0);
    mPairOfstream.write((char *)&mTotPairs, sizeof(int64));
}


/*
 *
 * Code for getting neighbor list
 *
 */



/*
 * Test to see if the source is in a usable quadrant for this lens.
 */
int32 LensSource::TestQuad(
        int32& bad12, 
        int32& bad23, 
        int32& bad34,
        int32& bad41, 
        double& theta)
{
   
	static const int32 UNMASKED=1, MASKED=0;

	// 1+2 or 3+4 are not masked
	if ( !bad12 || !bad34 ) {

		// keeping both quadrants
		if ( !bad12 && !bad34 ) {
			return(UNMASKED);
		}

		// only keeping one set of quadrants
		if (!bad12) {
			if (theta >= 0.0 && theta <= M_PI) {
				return(UNMASKED);
			} else {
				return(MASKED);
			}
		} else {
			if (theta >= M_PI && theta <= TWOPI) {
				return(UNMASKED);
			} else {
				return(MASKED);
			}
		}
	}

	// 2+3 or 4+1 are not masked
	if ( !bad23 || !bad41 ) {

		// keeping both quadrants
		if ( !bad23 && !bad41 ) {
			return(UNMASKED);
		}

		// only keeping one set of quadrants
		if (!bad23) {
			if (theta >= M_PI_2 && theta <= THREE_M_PI_2) {
				return(UNMASKED);
			} else {
				return(MASKED);
			}
		} else {
			if ( (theta >= THREE_M_PI_2 && theta <= TWOPI) ||
					(theta >= 0.0           && theta <= M_PI_2) ) {
				return(UNMASKED);
			} else {
				return(MASKED);
			}
		}

	}


}


/*
 *
 * Great circle distances and angles in survey coordinates
 *
 */

void LensSource::CalcGcircSurvey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double& dis,  double& theta)
{

  double sinlam1, coslam1, sinlam2, coslam2, 
    etadiff, cosetadiff, sinetadiff, cosdis;
  double tlam1, tlam2;

  tlam1 = lam1*D2R;
  //sinlam1 = sin(tlam1);
  coslam1 = cos(tlam1);
  sinlam1 = sqrt(1.0-coslam1*coslam1);
  if (tlam1 < 0) sinlam1 = -sinlam1;

  tlam2 = lam2*D2R;
  //sinlam2 = sin(tlam2);
  coslam2 = cos(tlam2);
  sinlam2 = sqrt(1.0-coslam2*coslam2);
  if (tlam2 < 0) sinlam2 = -sinlam2;

  etadiff = (eta2-eta1)*D2R;
  cosetadiff = cos(etadiff);
  sinetadiff = sqrt(1.0-cosetadiff*cosetadiff);
  if (etadiff < 0) sinetadiff=-sinetadiff;

  cosdis = sinlam1*sinlam2 + coslam1*coslam2*cosetadiff;

  if (cosdis < -1.0) cosdis=-1.0;
  if (cosdis >  1.0) cosdis= 1.0;

  dis = acos(cosdis);

  theta = atan2( sinetadiff, 
  		 (sinlam1*cosetadiff - coslam1*sinlam2/coslam2) ) - M_PI_2;

}


/*
 *
 * Intersect ra/dec with HTM triangles within angle cosangle. Fill idlist
 * with the HTM triangle ids
 *
 */
void LensSource::IntersectHtm(
        double rRa, 
        double rDec, 
        double rCosAngle, 
        vector<uint64> &rIdList)
{

    ValVec<uint64> plist, flist;
    uint64 id;

    // We must intitialize each time because it remembers it's state
    // internally
    SpatialDomain domain;    // initialize empty domain

    domain.setRaDecD(rRa,rDec,rCosAngle); //put in ra,dec,d E.S.S.

    domain.intersect(mSpatialIndex, plist, flist);	  // intersect with list

    rIdList.clear();
    for (int32 i=0; i<flist.length(); i++) {
        id = flist[i];
        if (id >= mPars.min_htm_index && id <= mPars.max_htm_index) {
            rIdList.push_back(id);
        }
    }
    for (int32 i=0; i<plist.length(); i++) 
    {
        id = plist[i];
        if (id >= mPars.min_htm_index && id <= mPars.max_htm_index) 
        {
            rIdList.push_back(id);
        }
    }

}

/*
 *
 * Get the list of sources that are within the specified htm id list
 *
 */

void LensSource::FindHtmSources(vector<uint64> &rIdList, vector<int> &rSlist)
{
    rSlist.clear();

    // Now extract the sources that are within these triangles
    for (int32 i=0; i<rIdList.size(); i++) 
    {
        // Convert leafid into a bin number
        int32 leafbin = rIdList[i] - mPars.min_htm_index;

        // any objects in this bin?
        if (mRev[leafbin] != mRev[leafbin+1])
        {
            int32 nleafbin = mRev[leafbin+1] - mRev[leafbin];
            // Loop over sources in this leaf
            for (int32 si=0; si<nleafbin; si++)
            {
                int32 sii=mRev[ mRev[leafbin]+si ];
                rSlist.push_back(sii);
            } // sources in leaf               
        } // non-empty leaf
    } // loop over HTM id list
}

/*
 *
 * Get the source indices within rmin,rmax of the lens
 *
 */

void LensSource::FindNearbySources(int32 rLensIndex)
{
    vector<int> tslist;
    double sangle, stheta;

    mSindices.clear();
    mSrkpc.clear();
    mStheta.clear();
 
    double lra = mLenses[rLensIndex].ra;
    double ldec = mLenses[rLensIndex].dec;
    double angle = mLenses[rLensIndex].angmax*D2R;
    double cosangle = cos(angle);
    double llam = mLenses[rLensIndex].clambda;
    double leta = mLenses[rLensIndex].ceta;

    // First get the HTM triangle list
    vector<uint64> idlist;
    IntersectHtm(lra, ldec, cosangle, idlist);
    FindHtmSources(idlist, tslist);

    // Now extract the ones within actual radius
    for (int32 i=0; i< tslist.size(); i++)
    {
        int32 si=tslist[i];
        double slam = mSources[si].clambda;
        double seta = mSources[si].ceta;

        // gets separation in radians and theta in radians
        gcirc_survey(llam, leta, slam, seta, sangle, stheta);

        double rkpc = sangle*mLenses[rLensIndex].DL;
        if (mPars.comoving) rkpc = rkpc*(1+mLenses[rLensIndex].z);

        if (rkpc >= mPars.rmin && rkpc <= mPars.rmax 
                && rkpc > MINIMUM_ANGLE )
        {
            mSindices.push_back(si);
            mSrkpc.push_back(rkpc);
            mStheta.push_back(stheta);
        }
    } 
}

/*
 * Lensing calculations for this source-lens pair.
 */

void LensSource::CalculateSourceShear(
        int32 rSourceSubIndex, 
        int32 rLensIndex, 
        float rSigInv)
{

    int16 idebug=0;
    int32 source_index = mSindices[rSourceSubIndex];

    if (mDebug2) {cout<<"debug stop:"<<idebug<<endl;idebug++;fflush(stdout);}
    struct source_old* tscat = &mSources[source_index];
    if (mDebug2) {cout<<"debug stop:"<<idebug<<endl;idebug++;fflush(stdout);}

    float sig_inv2 = rSigInv*rSigInv;

    double rkpc = mSrkpc[rSourceSubIndex]; 
    double xrel=rkpc*cos(mStheta[rSourceSubIndex]);
    double yrel=rkpc*sin(mStheta[rSourceSubIndex]);
    if (mDebug2) {cout<<"debug stop:"<<idebug<<endl;idebug++;fflush(stdout);}

    // eta is flipped
    double diffsq = xrel*xrel - yrel*yrel;
    double xy = xrel*yrel;

    double rkpc_inv2 = 1.0/rkpc/rkpc;

    double cos2theta = diffsq*rkpc_inv2;
    double sin2theta = 2.0*xy*rkpc_inv2;

    // Tangential/45-degree rotated ellipticities
    double e1prime = -(tscat->e1*cos2theta + tscat->e2*sin2theta);
    double e2prime =  (tscat->e1*sin2theta - tscat->e2*cos2theta);

    // covariance
    double e1e2err = tscat->e1e2err;
    double e1e2err2 = e1e2err*e1e2err;
    if (e1e2err < 0) e1e2err2 = -e1e2err2;

    if (mDebug2) {cout<<"debug stop:"<<idebug<<endl;idebug++;fflush(stdout);}
    double e1e1err2 = 
        tscat->e1e1err*tscat->e1e1err;

    double e2e2err2 = 
        tscat->e2e2err*tscat->e2e2err;

    // Errors in tangential/ortho
    double etan_err2 = 
        e1e1err2*cos2theta*cos2theta + e2e2err2*sin2theta*sin2theta - 
        2.0*cos2theta*sin2theta*e1e2err2; 

    double shear_err2 = 0.25*(etan_err2 + SHAPENOISE2);

    double ortho_err2 = 
        e1e1err2*sin2theta*sin2theta + e2e2err2*cos2theta*cos2theta - 
        2.0*cos2theta*sin2theta*e1e2err2; 

    double orthoshear_err2 = 0.25*(ortho_err2 + SHAPENOISE2);

    // density contrast
    double denscont = e1prime/2.0/rSigInv;
    double densconterr2 = shear_err2/sig_inv2;

    double orthodenscont = e2prime/2.0/rSigInv;
    double orthodensconterr2 = orthoshear_err2/sig_inv2;

    if (mPars.comoving)
    {
        float zlens = mLenses[rLensIndex].z;
        float comov_fac2 = 1./pow(1+zlens, 2);
        float comov_fac4 = 1./pow(1+zlens, 4);

        denscont *= comov_fac2;
        densconterr2 *= comov_fac4;

        orthodenscont *= comov_fac2;
        orthodensconterr2 *= comov_fac4;
    }


    float sweight=0.0, osweight=0.0;
    if (densconterr2 > 0.0) {
        sweight = 1./densconterr2;
        osweight = 1./orthodensconterr2;
    } 

    if (mDebug2) {cout<<"debug stop:"<<idebug<<endl;idebug++;fflush(stdout);}
    // What kind of binning?
    int32 radbin;
    if (mPars.logbin)
    {
        double log_rkpc = log10(rkpc);
        radbin = (int) ( (log_rkpc-mPars.logRmin)/mPars.logBinsize );
    }
    else 
    {
        radbin = (int) ( (rkpc-mPars.rmin)/mPars.binsize );
    }


    if (mDebug2) {cout<<"debug stop: "<<idebug<<endl;idebug++;fflush(stdout);}
    if ( (sweight > 0.0) && (radbin >= 0) && (radbin < mPars.nbin) )
    {
        if (mDebug)
        {
            cout <<" "<<rLensIndex<<" "<<source_index<<" "
                <<tscat->e1<<" "<<tscat->e2<<" "
                <<sweight<<" "<<denscont<<" "<<densconterr2<<" "
                <<xrel<<" "<<yrel<<" "<<rSigInv<<" "<<radbin;
        }

        mTotPairs += 1;


        float tw = 1./(etan_err2 + SHAPENOISE2);
        float f_e = etan_err2*tw;
        float f_sn = SHAPENOISE2*tw;

        // coefficients (p 596 Bern02) 
        // there is a k1*e^2/2 in Bern02 because
        // its the total ellipticity he is using
        float wts_ssh = sweight;
        float k0 = f_e*SHAPENOISE2;
        float k1 = f_sn*f_sn;
        float F = 1. - k0 - k1*e1prime*e1prime;


        // keep running totals of positions for
        // ellipticity of source distribution
        mXpysum += xrel*xrel + yrel*yrel;
        mXmysum += diffsq;
        mXysum  += xy;



        ////////////////////////////////////////
        // Fill in the lens structure
        ////////////////////////////////////////

        mLensout.tot_pairs += 1;
        mLensout.npair[radbin] +=1;

        mRmaxAct = 
            mLensout.rmax_act[radbin];
        mRmaxAct =
            mLensout.rmin_act[radbin];

        if (mRmaxAct == 0.0) {
            mRmaxAct=rkpc;
        } else {
            mRmaxAct = max(mRmaxAct, (float32) rkpc);
        }
        if (mRmaxAct == 0.0) {
            mRmaxAct=rkpc;
        } else {
            mRmaxAct = min(mRmaxAct, (float32) rkpc);
        }

        mLensout.rmax_act[radbin] = mRmaxAct;
        mLensout.rmin_act[radbin] = mRmaxAct;

        // these are initally sums, then converted to
        // means later by dividing by wsum

        mLensout.rsum[radbin]     += rkpc;

        mLensout.sigma[radbin]    += sweight*denscont;
        mLensout.orthosig[radbin] += osweight*orthodenscont;

        mLensout.weight        += sweight;
        mLensout.wsum[radbin]  += sweight;
        mLensout.owsum[radbin] += osweight;
        mLensout.wscritinvsum[radbin] += sweight*rSigInv;

        mLensout.sigerrsum[radbin] += 
            sweight*sweight*denscont*denscont;
        mLensout.orthosigerrsum[radbin] += 
            osweight*osweight*orthodenscont*orthodenscont;

        mLensout.sshsum   += wts_ssh*F;
        mLensout.wsum_ssh += wts_ssh;

        if (mPars.dopairs)
        {
            WritePairs(rLensIndex, source_index, rkpc, sweight);
        }

    }// Non-zero weight?
    else if (mDebug) 
    {
        if (sweight <= 0.0)
            printf(" bad weight");
    }

    if (mDebug2) {cout<<"debug stop: "<<idebug<<endl;idebug++;fflush(stdout);}

}

void LensSource::ProcessSources(int32 rLensIndex)
{
    ResetLensout();
    mLensout.zindex = mLenses[rLensIndex].zindex;


    mTotPairs=0;
    float sig_inv;
    for (int32 i=0; i<mSindices.size(); i++) {

        int32 isrc = mSindices[i];
        float zsource = mSources[isrc].z;
        float zlens = mLenses[rLensIndex].z;

        // Measure the appropriate sigma crit
        if (mPars.sigmacrit_style == 1)
        {
            if ( zsource >= (zlens + mPars.zbuffer) )
            {
                /*
                float adiff_ls = mAetaRelLens[rLensIndex] - mAetaRelSource[isrc];
                float adiff_s = mAetaZero - mAetaRelSource[isrc];
                float DL = mLenses[rLensIndex].DL/1000.0;
                sig_inv = mScinvFactor*DL*adiff_ls/adiff_s;
                */

                float DL = mLenses[rLensIndex].DL/1000.0;
                sig_inv = 
                    sigmaCritInv(
                            DL,
                            mAetaRelLens[rLensIndex],
                            mAetaRelSource[isrc]);
            }
            else {
                sig_inv = -9999.0;
            }

        }
        else if (mPars.sigmacrit_style == 2)
        {
            // Using mean inverse critical density, integrated over
            // deconvolved distribution.  Note: this is not changing
            // for each source, but the assignment is placed here
            // anyway for clarity
            sig_inv = mLenses[rLensIndex].mean_scinv;

        }
        else if (mPars.sigmacrit_style == 3) 
        {
            sig_inv = sigmaCritInvInterp2d(zlens, zsource, &mScinv);
        }

        if (sig_inv > 0.0)
        {
            CalculateSourceShear(i, rLensIndex, sig_inv);
            if (mDebug) printf("\n");

        }// Good sig_inv?
        else if (mDebug) 
        {
            printf(" bad siginv.  zlens: %f zsource: %f sig_inv = %e\n", 
                    zlens, zsource, sig_inv);
        }

    }

}



/*
 * Process the sources.  Calculate the inverse critical density and if
 * good bin up, calculate shear, and add the result.
 */
void LensSource::ProcessSourcesQuad(int32 rLensIndex)
{
    ResetLensout();
    mLensout.zindex = mLenses[rLensIndex].zindex;

    int32 pixelMaskFlags = mLenses[rLensIndex].pixelMaskFlags;
    int32 bad12 = pixelMaskFlags & (FLAGS_QUAD1_MASKED+FLAGS_QUAD2_MASKED);
    int32 bad23 = pixelMaskFlags & (FLAGS_QUAD2_MASKED+FLAGS_QUAD3_MASKED);  
    int32 bad34 = pixelMaskFlags & (FLAGS_QUAD3_MASKED+FLAGS_QUAD4_MASKED);
    int32 bad41 = pixelMaskFlags & (FLAGS_QUAD4_MASKED+FLAGS_QUAD1_MASKED);

    mTotPairs=0;
    float sig_inv;
    for (int32 i=0; i<mSindices.size(); i++)
    {

        int32 isrc = mSindices[i];
        float zsource = mSources[isrc].z;
        float zlens = mLenses[rLensIndex].z;


        double theta2 = M_PI_2 - mStheta[i];
        if (TestQuad(bad12,bad23,bad34,bad41,theta2))
        {
            // Measure the appropriate sigma crit
            if (mPars.sigmacrit_style == 1)
            {
                if ( zsource >= (zlens + mPars.zbuffer) )
                {
                    float adiff_ls = mAetaRelLens[rLensIndex] - mAetaRelSource[isrc];
                    float adiff_s = mAetaZero - mAetaRelSource[isrc];
                    float DL = mLenses[rLensIndex].DL/1000.0;
                    sig_inv = mScinvFactor*DL*adiff_ls/adiff_s;
                }
                else {
					sig_inv = -9999.0;
				}

            }
            else if (mPars.sigmacrit_style == 2)
            {
                // Using mean inverse critical density, integrated over
                // deconvolved distribution.  Note: this is not changing
                // for each source, but the assignment is placed here
                // anyway for clarity
                sig_inv = mLenses[rLensIndex].mean_scinv;

            }
            else if (mPars.sigmacrit_style == 3) 
            {
                sig_inv = sigmaCritInvInterp2d(zlens, zsource, &mScinv);
            }

            if (sig_inv > 0.0)
            {
                CalculateSourceShear(i, rLensIndex, sig_inv);
                if (mDebug) printf("\n");

            }// Good sig_inv?
            else if (mDebug) 
            {
                printf(" bad siginv.  zlens: %f zsource: %f sig_inv = %e\n", 
                        zlens, zsource, sig_inv);
            }

        }
    }

}


/*
 * Averages. Computed if the measurements are good
 */
void LensSource::ComputeLensAverages()
{
    double ie1 = mXmysum/mXpysum;
    double ie2 = 2.*mXysum/mXpysum;
    double ie = sqrt( ie1*ie1 + ie2*ie2 );
    mLensout.ie = ie;

    double mm = 3.0/sqrt(mLensout.tot_pairs);

    if ( (mLensout.weight > 0.0) && (ie < max(mm, mMaxe)) )
    {
        for (int32 radbin=0; radbin<mPars.nbin; radbin++)
        {
            if (mLensout.wsum[radbin] > 0.0) 
            {
                float inverse_wsum = 1.0/( mLensout.wsum[radbin] );
                float inverse_owsum = 1.0/( mLensout.owsum[radbin] );

                float tsigwsum = mLensout.wsum[radbin];
                float tsigsum  = mLensout.sigma[radbin];
                if (!isnan(tsigwsum) 
                        && !isnan(tsigsum) 
                        && !isnan(inverse_wsum)) 
                {
                    mSigwsum += tsigwsum;
                    mSigsum += tsigsum;
                    mLensout.sigma[radbin] *= inverse_wsum;
                    mLensout.sigmaerr[radbin] = sqrt(inverse_wsum);

                    mLensout.orthosig[radbin] *= inverse_owsum;
                    mLensout.orthosigerr[radbin] = sqrt(inverse_owsum);

                } else {
                    mLensout.wsum[radbin] = 0.0;
                    mLensout.sigma[radbin] = 0.0;
                    mLensout.sigmaerr[radbin] = 0.0;

                    mLensout.owsum[radbin] = 0.0;
                    mLensout.orthosig[radbin] = 0.0;
                    mLensout.orthosigerr[radbin] = 0.0;

                    cout<<"nan: "<<inverse_wsum<<" "
                        <<mLensout.sigma[radbin]<<" "
                        <<mLensout.sigmaerr[radbin]<<endl;
                    printf("/");
                }

            }
        }
    }           
    else
    {

        mLensout.ie = 9999.0;
        for (int32 radbin=0; radbin<mPars.nbin; radbin++)
        {
            mLensout.wsum[radbin] = 0.0;
            mLensout.sigma[radbin] = 0.0;
            mLensout.sigmaerr[radbin] = 0.0;

            mLensout.owsum[radbin] = 0.0;
            mLensout.orthosig[radbin] = 0.0;
            mLensout.orthosigerr[radbin] = 0.0;
        }

        //cout<<"\\"<<endl;
    }


}



/*
 * Print some updates
 */

void LensSource::PrintMeans(int32 rLensIndex)
{

  if (mSigwsum > 0) 
    {
      float mean_denscont = mSigsum/mSigwsum;
      float mean_denscont_err = sqrt(1.0/mSigwsum);
      printf("\nlens = %d/%d   Mean dens. cont. = %f +/- %f\n",
	     rLensIndex,mPars.nlens,
	     mean_denscont,mean_denscont_err);		    
    }
  else 
    {
      printf("\nlens = %d/%d   Mean dens. cont. = 0 +/- 0\n",
	     rLensIndex,mPars.nlens);		    
    }
}


void LensSource::DoTheMeasurement()
{

    MakeLensout();
    mSigwsum = mSigsum = 0;
    mMaxe=0.2;

    //mFilePtr = fopen(mPars.output_file.c_str(), "w");
    mOfstream.open(mPars.output_file.c_str());
    WriteHeader();

    if (mPars.dopairs)
    {
        mPairOfstream.open(mPars.pair_file.c_str());
        WritePairHeader();
    }

    if ( (mPars.zbuffer != 0.0) 
            && (mPars.sigmacrit_style == 2) )
    {
        cout << "Do not support zbuffer for sigmacrit_style==2 yet" << endl;
        exit(1);
    }

    int32 step=500;
    int32 big_step=10000;
    cout << endl <<
        "Each dot is " << step << " lenses" << endl;


    //mPars.nlens=200;
    for (int32 lens_index=0; lens_index<mPars.nlens; lens_index++)
    {
        // This fills class variables.
        FindNearbySources(lens_index);

        // Initialize the x^2-y^2, x^2+y^2, xy sums
        mXpysum = mXmysum = mXysum = 0;

        // Process all the sources, keeping sums in mLensout
        ProcessSources(lens_index);

        // Compute averages (dividing by weights)
        ComputeLensAverages();

        // Output the result even for "bad" measurements.
        WriteLensout();


        // Progress reports
        if ( (lens_index % step) == 0 && (lens_index != 0)) 
        {
            printf(".");
            if ( (lens_index % big_step) == 0)
                PrintMeans(lens_index);
            fflush(stdout);
        }

    }

    //fclose(mFilePtr);
    mOfstream.close();

    if (mPars.dopairs)
    {
        WriteTotPairs();
        mPairOfstream.close();
    }
    cout << endl << "Done" << endl;
}
