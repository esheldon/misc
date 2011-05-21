#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"
#include "fstream.h"

#include <iostream>
#include <sstream>
#include <string>

#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <algorithm>

#include "lensout.h"
#include "lensutil.h"
#include "lens_constants.h"
#include "types.h"
#include "lens_source.h"
#include "gcircSurvey.h"
#include "sigmaCritInv.h"

#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{

  if (argc < 2)  
    {
      cout << "-Syntax: objshear par_file" << endl;
      return(1);
    }

  char *par_file = argv[1];

  LENS_SOURCE data(par_file);

  /* Copy out pointers to the data */
  lens_struct   *lcat = data.lcat();
  source_pixel_struct *scat = data.scat_pixel();
  source_pixel_struct *tscat;
  int32         *revInd = data.rev();
  PAR_STRUCT    *par_struct = data.par();

  float32 scinv_factor = data.scinv_factor();
  float32 aeta_zero    = data.aeta_zero();
  float32 *aeta_lens   = data.aeta_lens();

  // Need to fill this in
  SCINV_STRUCT scinvStruct;  

  float32 H0 = par_struct->h*100.0;
  float32 omega_m = par_struct->omega_m;

  int32 minLeafId = par_struct->min_htm_index;
  int32 maxLeafId = par_struct->max_htm_index;

  int32 step=500;
  int32 bigStep=10000;

  // Number of lenses
  int32 i;
  int32 nregion=0;
  for (i=0; i<data.nlens();i++)
    if (lcat[i].region == par_struct->region)
      nregion++;
  par_struct->nlens_region = nregion;

  cout << endl <<
    "Number of lenses in this region: " << nregion << endl;
  cout << endl <<
    "Each dot is " << step << " lenses" << endl;

  lensout_struct lensout;
  make_lensout(lensout, par_struct->nbin);

  FILE *fptr = fopen(par_struct->output_file, "w");
  cout << "Writing header for file " << par_struct->output_file << endl;
  write_header(fptr, par_struct);

  try {

    // construct index with given depth and savedepth
    int32 savedepth=2;
    int32 nlens = data.nlens();

    htmInterface htm(par_struct->depth,savedepth);  // generate htm interface
    const SpatialIndex &index = htm.index();

    /* keeping track of those found */
    int64 nFoundTot=0, nFoundUse=0, totalPairs=0, badBin=0;
    float32 sigwsum=0, sigsum=0;

    // Loop over all the lenses

    printf("lens = 0/%d   Mean dens. cont. = 0 +/- 0\n",
	   nregion);
    fflush(stdout);

    int32 lensUsed = 0,lensUsedOld=0;
    for (int32 lensIndex=0; lensIndex < nlens; lensIndex++) 
      {

	lensout.zindex = lcat[lensIndex].zindex;

	if (lcat[lensIndex].region == par_struct->region) 
	  {
	    int16 bad;
	    int16 pixelMaskFlags;
      
	    lensUsed++;

	    pixelMaskFlags = lcat[lensIndex].pixelMaskFlags;

	    bad = pixelMaskFlags & FLAGS_MASKED;


	    // Is the central point masked?  I often check before
	    // making the catalog...
	    if (!bad) 
	      {

		
		// Declare the domain and the lists
		SpatialDomain domain;    // initialize empty domain
		ValVec<uint64> plist, flist;	// List results

		vector <uint32> idList;
		int32 idCount=0;

		static const float 
		  PI=3.1415927,
		  TWOPI=6.2831853,
		  PIOVER2=1.5707963,
		  THREE_PIOVER2=4.7123890;
	    
		float32 
		  zlens, zSource, zSourceErr, 
		  sig_inv, sig_inv2, 
		  sWeight, osWeight, wts_ssh,
		  inverse_wsum, rmin_act, rmax_act;

		float32 comov_fac2, comov_fac4;
		
		float64 
		  ra, dec, clambda, ceta;
		
		float64 d, R, theta, theta2, Rkpc;
		float32 DL;
		float32 adiff_ls, adiff_s;

		int8 radBin;
		
		float64 
		  xrel, yrel, diffsq, xy, 
		  xpysum=0, xmysum=0, xysum=0, ie1, ie2, ie, mm, maxe=0.2, 
		  RkpcInv2, cos2theta, sin2theta,
		  e1prime, e2prime, e1e1err2, e1e2err, e1e2err2, e2e2err2,
		  etan_err2, shear_err2, ortho_err2, orthoshear_err2,
		  denscont, densconterr2, orthodenscont, orthodensconterr2;
		
		float32 tw, f_e, f_sn, k0, k1, F;
		
		int16 bad12, bad23, bad34, bad41;
		
		int32 i, j, n, iSrc, srcUse;
		int32 nFound;
		int32 leafId, leafBin, nLeafBin;
		float32 logRkpc;

		bad12 = pixelMaskFlags & (FLAGS_QUAD1_MASKED+FLAGS_QUAD2_MASKED);
		bad23 = pixelMaskFlags & (FLAGS_QUAD2_MASKED+FLAGS_QUAD3_MASKED);  
		bad34 = pixelMaskFlags & (FLAGS_QUAD3_MASKED+FLAGS_QUAD4_MASKED);
		bad41 = pixelMaskFlags & (FLAGS_QUAD4_MASKED+FLAGS_QUAD1_MASKED);

		// extract some shorthand 
		ra      = lcat[lensIndex].ra;
		dec     = lcat[lensIndex].dec;
		clambda = lcat[lensIndex].clambda;
		ceta    = lcat[lensIndex].ceta;

		zlens = lcat[lensIndex].z;

		d = cos( lcat[lensIndex].angmax*D2R );

		domain.setRaDecD(ra,dec,d); //put in ra,dec,d E.S.S.

		domain.intersect(&index,plist,flist);	  // intersect with list
    
		
		nFound = flist.length() + plist.length();
		nFoundTot += nFound;
    
		// Save the result in idlist. This is not a bottleneck
		idList.resize(nFound);

		// ----------- FULL NODES -------------
		for(i = 0; i < flist.length(); i++)
		  {  
		    idList[idCount] = (uint32 )flist(i);
		    idCount++;
		  }
		// ----------- Partial Nodes ----------
		for(i = 0; i < plist.length(); i++)
		  {  
		    idList[idCount] = (uint32 )plist(i);
		    idCount++;
		  }
		
      
		// Now loop over leaf ids
		for(i=0; i<nFound;i++)
		  {

		    
		    leafId = idList[i];

		    // Convert leafid into bin number
		    leafBin = idList[i] - minLeafId;

		    
		    // Check if there are sources in this leafid
		    if ( leafId >= minLeafId  &&  leafId <= maxLeafId)
		      {


			// Does this triangle have data in or catalog?
			if (revInd[leafBin] != revInd[leafBin+1]) 
			  {

			    
			    nFoundUse+=1;

			    srcUse = revInd[ revInd[leafBin]+iSrc ];

			    tscat = &scat[srcUse];

			    // Distance and angle to source
			    // *** This is a major bottleneck ***
			    //gcircSurvey(clambda, ceta, 
			    //	    tscat->clambda, tscat->ceta,
			    //	    R, theta);
			    gcircSurvey2(clambda, ceta, 
					 tscat->clambda, tscat->ceta,
					 R, theta);
				
			    // kpc
			    DL = lcat[lensIndex].DL;
			    Rkpc = R*DL;
			    
			    // Mpc
			    DL = DL/1000.0;
			    
			    // convert to comoving?
			    if (par_struct->comoving) 
			      {
				comov_fac2 = 1./pow(1+zlens, 2);
				comov_fac4 = 1./pow(1+zlens, 4);
				Rkpc = Rkpc*(1+zlens);
			      }

			    // Within our circular radius?
			    if (Rkpc >= par_struct->rmin && Rkpc <= par_struct->rmax)
			      {
				
				// Is this source in one of the quadrants that is 
				// useful for this lens?
				theta2 = PIOVER2 - theta;
				if (objShearTestQuad(bad12,bad23,bad34,bad41,theta2))
				  {
					
				    // What kind of binning?
				    if (par_struct->logbin)
				      {
					logRkpc = log10(Rkpc);
					radBin = (int8) ( (logRkpc-par_struct->logRmin)/par_struct->logBinsize );
				      }
				    else 
				      {
					radBin = (int8) ( (Rkpc-par_struct->rmin)/par_struct->binsize );
				      }

					
				    // valid bin number?
				    if (radBin >= 0 && radBin < par_struct->nbin)
				      {

					  
					// Mean inverse critical density for this pixel/lens
					sig_inv = 
					  scinv_factor*DL*(aeta_lens[lensIndex]*tscat->aeta1 - tscat->aeta2);
					    
					if (sig_inv > 0.0) 
					  {
					    
					    totalPairs +=1;
					    
					    sig_inv2 = sig_inv*sig_inv;
					    
					    // X/Y positions
					    xrel = Rkpc*cos(theta);
					    yrel = Rkpc*sin(theta);
					    
					    // eta is flipped
					    diffsq = xrel*xrel - yrel*yrel;
					    xy = xrel*yrel;
					    
					    RkpcInv2 = 1.0/Rkpc/Rkpc;
					    
					    cos2theta = diffsq*RkpcInv2;
					    sin2theta = 2.0*xy*RkpcInv2;
				    
					    // Tangential/45-degree rotated ellipticities
					    e1prime = -(tscat->e1*cos2theta + tscat->e2*sin2theta);
					    e2prime =  (tscat->e1*sin2theta - tscat->e2*cos2theta);



					    // covariance
					    e1e2err = tscat->e1e2err;
					    e1e2err2 = e1e2err*e1e2err;
					    if (e1e2err < 0) e1e2err2 = -e1e2err2;
				    
					    e1e1err2 = 
					      tscat->e1e1err*tscat->e1e1err;

					    e2e2err2 = 
					      tscat->e2e2err*tscat->e2e2err;


					    // Errors in tangential/ortho
					    etan_err2 = 
					      e1e1err2*cos2theta*cos2theta + e2e2err2*sin2theta*sin2theta - 
					      2.0*cos2theta*sin2theta*e1e2err2; 

					    shear_err2 = 0.25*(etan_err2 + SHAPENOISE2);

					    ortho_err2 = 
					      e1e1err2*sin2theta*sin2theta + e2e2err2*cos2theta*cos2theta - 
					      2.0*cos2theta*sin2theta*e1e2err2; 
				    
					    orthoshear_err2 = 0.25*(ortho_err2 + SHAPENOISE2);
				    
					    // density contrast
					    denscont = e1prime/2.0/sig_inv;
					    densconterr2 = shear_err2/sig_inv2;
				    
					    orthodenscont = e2prime/2.0/sig_inv;
					    orthodensconterr2 = orthoshear_err2/sig_inv2;

					    if (par_struct->comoving) 
					      {
						denscont *= comov_fac2;
						densconterr2 *= comov_fac4;
						
						orthodenscont *= comov_fac2;
						orthodensconterr2 *= comov_fac4;
					      }

					    sWeight = 1./densconterr2;
					    osWeight = 1./orthodensconterr2;

					    tw = 1./(etan_err2 + SHAPENOISE2);
					    f_e = etan_err2*tw;
					    f_sn = SHAPENOISE2*tw;
				    
					    // coefficients (p 596 Bern02) 
					    // there is a k1*e^2/2 in Bern02 because
					    // its the total ellipticity he is using

					    wts_ssh = sWeight;
					    k0 = f_e*SHAPENOISE2;
					    k1 = f_sn*f_sn;
					    F = 1. - k0 - k1*e1prime*e1prime;
				
					    // keep running totals of positions for
					    // ellipticity of source distribution
				
					    xpysum += xrel*xrel + yrel*yrel;
					    xmysum += diffsq;
					    xysum  += xy;
				
					    ////////////////////////////////////////
					    // Fill in the lens structure
					    ////////////////////////////////////////
					    
					    lensout.tot_pairs += 1;
					    lensout.npair[radBin] +=1;
					    
					    rmax_act = 
					      lensout.rmax_act[radBin];
					    rmin_act =
					      lensout.rmin_act[radBin];
					    
					    if (rmax_act == 0.0) {
					      rmax_act=Rkpc;
					    } else {
					      rmax_act = max(rmax_act, (float32) Rkpc);
					    }
					    if (rmin_act == 0.0) {
					      rmin_act=Rkpc;
					    } else {
					      rmin_act = min(rmin_act, (float32) Rkpc);
					    }
				
					    lensout.rmax_act[radBin] = rmax_act;
					    lensout.rmin_act[radBin] = rmin_act;
					    
					    // these are initally sums, then converted to
					    // means later by dividing by wsum
				
					    lensout.rsum[radBin]     += Rkpc;
					    
					    lensout.sigma[radBin]    += sWeight*denscont;
					    lensout.orthosig[radBin] += osWeight*orthodenscont;

					    lensout.weight        += sWeight;
					    lensout.wsum[radBin]  += sWeight;
					    lensout.owsum[radBin] += osWeight;
				
					    lensout.sigerrsum[radBin] += 
					      sWeight*sWeight*denscont*denscont;
					    lensout.orthosigerrsum[radBin] += 
					      osWeight*osWeight*orthodenscont*orthodenscont;
				
					    lensout.sshsum   += wts_ssh*F;
					    lensout.wsum_ssh += wts_ssh;
					    
					  } // Good sig_inv?
					    
				      } 
				    else // good radBin? 
				      {
					badBin += 1;
				      }
					
				  } // Good quadrant?
				    
			      } // radius within circular radius min/max?

			  } // any sources found in this leaf?

		      } // leaf id is in allowed range?

		  } // loop over found leaf ids


		/////////////////////////////////////////////////////////
		// Now compute averages if there were any sources for
		// this lens, i.e. weight > 0
		/////////////////////////////////////////////////////////

		if (lensout.weight > 0.0) 
		  {
	  
		    ie1 = xmysum/xpysum;
		    ie2 = 2.*xysum/xpysum;
		    ie = sqrt( ie1*ie1 + ie2*ie2 );
		    lensout.ie = ie;

		    mm = 3.0/sqrt(lensout.tot_pairs);

		    for(radBin=0; radBin<par_struct->nbin; radBin++)
		      {

			if (lensout.wsum[radBin] > 0.0) 
			  {

			    // Keep track for "good" lenses
			    if (ie < max(mm, maxe)) 
			      {
				sigwsum += lensout.wsum[radBin];
				sigsum  += lensout.sigma[radBin];
			      }

			    inverse_wsum = 1.0/( lensout.wsum[radBin] );
		  
			    lensout.sigma[radBin] *= inverse_wsum;
			    lensout.sigmaerr[radBin] = sqrt(inverse_wsum);

			  }
			if (lensout.owsum[radBin] > 0.0) 
			  {
			    inverse_wsum = 1.0/( lensout.owsum[radBin] );
		  
			    lensout.orthosig[radBin] *= inverse_wsum;
			    lensout.orthosigerr[radBin] = sqrt(inverse_wsum);
			  }
			
		      }
		    
		  } // weight > 0.0?
			     
	      } // central point unmasked?
	
	    // need to write all in region since we set NROWS = nregion
	    // For large angles, all lenses have weight > 0 but for smaller
	    // radii this may write zeroes
	    write_lensout(fptr, lensout);

	  } // in region?

	reset_lensout(lensout);

	/*
	if ( (lensIndex % step) == 0 && (lensIndex != 0)) 
	  {
	    printf(".");
	    if ( (lensIndex % bigStep) == 0)
	      objShearPrintMeans(lensUsed,nregion,sigsum,sigwsum);
	    fflush(stdout);
	  }
	*/
	if ( (lensUsed % step) == 0 && (lensUsed != 0) && 
	     (lensUsed != lensUsedOld) ) 
	  {
	    printf(".");
	    if ( (lensUsed % bigStep) == 0)
	      objShearPrintMeans(lensUsed,nregion,sigsum,sigwsum);
	    fflush(stdout);
	    lensUsedOld = lensUsed;
	  }

      } // loop over lenses 

  
  
  } catch (SpatialException &x) {
    printf("%s\n",x.what());
  }



  fclose(fptr);

  cout << endl << "Done" << endl;

  return(0);
}

