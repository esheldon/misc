#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"
#include "fstream.h"
#include <time.h>
#include "export.h"
#include "objShear.h"
#include "sigmaCritInv.h"
#include "gcircSurvey.h"
#include <vector>
#include <algorithm>
using namespace std;


int
objShear(LENS_INPUT_STRUCT *lensInStruct, 
	 LRG_SCAT *scat, 
	 int32 *revInd,
	 SCINV_STRUCT *scinvStruct,
	 PAR_STRUCT *parStruct) 

{


  int32 nFound;
  int64 nFoundTot=0, nFoundUse=0, totalPairs=0;
  int32 savedepth;
    
  int32 lensIndex;
  
  // working in leaves
  int32 minLeafId, maxLeafId, leafId, leafBin, nLeafBin;

  float32 H0, omegaMatter;

  float32 logRmin, logRmax, logBinsize, logRkpc;

  int32 step=500;
  int32 bigStep=10000;

  float32 sigwsum=0, sigsum=0;	      

  savedepth=2;

  // Cannot use more than NBIN_MAX bins
  if (parStruct->nbin != NBIN_MAX)
    {
      printf("ERROR: parStruct->nbin = %d is larger than NBIN_MAX = %d\n",
	     parStruct->nbin, NBIN_MAX);
      return(1);
    }

  // What type of binning?
  if (parStruct->logbin)
    {
      logRmin = log10(parStruct->rmin);
      logRmax = log10(parStruct->rmax);

      logBinsize = ( logRmax - logRmin )/parStruct->nbin;
    }

  // assuming that source catalog is sorted by leafid
  minLeafId = scat[0].leafId;
  maxLeafId = scat[parStruct->nscat-1].leafId;

  H0 = parStruct->h*100.0;
  omegaMatter = parStruct->omegaMatter;

  // print something to show we are alive
  printf("\n");
  printf("Using depth   = %d\n", parStruct->depth);
  printf("nLensInStruct = %d\n", parStruct->nLensInStruct);
  printf("Rmin          = %f\n", parStruct->rmin);
  printf("Rmax          = %f\n", parStruct->rmax);

  printf("minLeafId     = %d\n", minLeafId);
  printf("maxLeafId     = %d\n", maxLeafId);

  printf("logbin        = %d\n", parStruct->logbin);  
  printf("interpPhotoz  = %d\n", parStruct->interpPhotoz);
  printf("nbin          = %d\n", parStruct->nbin);

  printf("H0            = %f\n", H0);
  printf("omegaMatter   = %f\n", parStruct->omegaMatter);

  printf("\nEach Dot is %d lenses\n\n",step);
  fflush(stdout);

  try {
    // construct index with given depth and savedepth
    htmInterface htm(parStruct->depth,savedepth);  // generate htm interface
    const SpatialIndex &index = htm.index();

    // Loop over all the lenses

    printf("lens = 0/%d   Mean dens. cont. = 0 +/- 0\n",
	   parStruct->nLensInStruct);
    for (lensIndex=0;lensIndex<parStruct->nLensInStruct;lensIndex++) 
      {

	int16 bad;
	int16 pixelMaskFlags;
      
	// Flags for this lens
	pixelMaskFlags = lensInStruct[lensIndex].pixelMaskFlags;
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

	    float64 
	      ra, dec, clambda, ceta;

	    float64 d, R, theta, theta2, Rkpc;

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

	    bad12 = pixelMaskFlags & (FLAGS_QUAD1_MASKED+FLAGS_QUAD2_MASKED);
	    bad23 = pixelMaskFlags & (FLAGS_QUAD2_MASKED+FLAGS_QUAD3_MASKED);  
	    bad34 = pixelMaskFlags & (FLAGS_QUAD3_MASKED+FLAGS_QUAD4_MASKED);
	    bad41 = pixelMaskFlags & (FLAGS_QUAD4_MASKED+FLAGS_QUAD1_MASKED);

	    // extract some shorthand 
	    ra      = lensInStruct[lensIndex].ra;
	    dec     = lensInStruct[lensIndex].dec;
	    clambda = lensInStruct[lensIndex].clambda;
	    ceta    = lensInStruct[lensIndex].ceta;

	    zlens = lensInStruct[lensIndex].z;
	
	    d = cos( lensInStruct[lensIndex].angMax*D2R );

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
      
	    // Now loop over these ids and get the sources
	    for(i=0; i<nFound;i++)
	      {

		leafId = idList[i];

		// Convert leafid into bin number
		leafBin = idList[i] - minLeafId;

		// Check if there are sources in this leafid
		if ( leafId >= minLeafId  &&  leafId <= maxLeafId)
		  {
		    // Look for sources in this triangle
		    if (revInd[leafBin] != revInd[leafBin+1]) 
		      {
			nFoundUse+=1;

			nLeafBin = revInd[leafBin+1] - revInd[leafBin];

			// Loop over sources in this leaf
			for(iSrc=0;iSrc<nLeafBin;iSrc++)
			  {

			    srcUse = revInd[ revInd[leafBin]+iSrc ];

			    // Distance and angle to source
			    gcircSurvey(clambda, ceta, 
					scat[srcUse].clambda, scat[srcUse].ceta,
					R, theta);

			    Rkpc = R*lensInStruct[lensIndex].DL;

			    // Within our circular radius?
			    if (Rkpc >= parStruct->rmin && Rkpc <= parStruct->rmax)
			      {

				// Is this source in one of the quadrants that is 
				// useful for this lens?
				theta2 = PIOVER2 - theta;
				if (objShearTestQuad(bad12,bad23,bad34,bad41,theta2))
				  {

				    // What kind of binning?
				    if (parStruct->logbin)
				      {
					logRkpc = log10(Rkpc);
					radBin = (int8) ( (logRkpc-logRmin)/logBinsize );
				      }
				    else 
				      {
					radBin = (int8) ( (Rkpc-parStruct->rmin)/parStruct->binsize );
				      }

				    // valid bin number?
				    if (radBin >= 0 && radBin < parStruct->nbin)
				      {
				  
					// This is actually a bottleneck after the
					// searching
					if (parStruct->interpPhotoz) 
					  {
					    zSource = scat[srcUse].photoz_z;
					    zSourceErr = scat[srcUse].photoz_zerr;
					    sig_inv = sigmaCritInv(H0, omegaMatter, 
								   zlens, zSource, zSourceErr, 
								   scinvStruct);
					  }
					else
					  {
					    zSource = scat[srcUse].photoz_z;
					    sig_inv = sigmaCritInv(H0, omegaMatter, 
								   zlens, zSource);
					  }
				  
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
					    e1prime = -(scat[srcUse].e1*cos2theta + scat[srcUse].e2*sin2theta);
					    e2prime =  (scat[srcUse].e1*sin2theta - scat[srcUse].e2*cos2theta);



					    // covariance
					    e1e2err = scat[srcUse].e1e2err;
					    e1e2err2 = e1e2err*e1e2err;
					    if (e1e2err < 0) e1e2err2 = -e1e2err2;
				    
					    e1e1err2 = 
					      scat[srcUse].e1e1err*scat[srcUse].e1e1err;

					    e2e2err2 = 
					      scat[srcUse].e2e2err*scat[srcUse].e2e2err;


					    // Errors in tangential/ortho
					    etan_err2 = 
					      e1e1err2*cos2theta*cos2theta + 
					      e2e2err2*sin2theta*sin2theta - 
					      2.0*cos2theta*sin2theta*e1e2err2; 

					    shear_err2 = 0.25*(etan_err2 + SHAPENOISE2);

					    ortho_err2 = 
					      e1e1err2*sin2theta*sin2theta + 
					      e2e2err2*cos2theta*cos2theta - 
					      2.0*cos2theta*sin2theta*e1e2err2; 
				    
					    orthoshear_err2 = 0.25*(ortho_err2 + SHAPENOISE2);
				    
					    // density contrast
					    denscont = e1prime/2.0/sig_inv;
					    densconterr2 = shear_err2/sig_inv2;
				    
					    orthodenscont = e2prime/2.0/sig_inv;
					    orthodensconterr2 = orthoshear_err2/sig_inv2;
				    
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
				
				
					    lensInStruct[lensIndex].totPairs += 1;
					    lensInStruct[lensIndex].npair[radBin] +=1;
				
					    rmax_act = 
					      lensInStruct[lensIndex].rmax_act[radBin];
					    rmin_act =
					      lensInStruct[lensIndex].rmin_act[radBin];
				
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
				
					    lensInStruct[lensIndex].rmax_act[radBin] = rmax_act;
					    lensInStruct[lensIndex].rmin_act[radBin] = rmin_act;
				
					    // these are initally sums, then converted to
					    // means later by dividing by wsum
				
					    lensInStruct[lensIndex].rsum[radBin]     += Rkpc;
				
					    lensInStruct[lensIndex].sigma[radBin]    += sWeight*denscont;
					    lensInStruct[lensIndex].orthosig[radBin] += osWeight*denscont;

					    lensInStruct[lensIndex].weight        += sWeight;
					    lensInStruct[lensIndex].wsum[radBin]  += sWeight;
					    lensInStruct[lensIndex].owsum[radBin] += osWeight;
				
					    lensInStruct[lensIndex].sigerrsum[radBin] += 
					      sWeight*sWeight*denscont*denscont;
					    lensInStruct[lensIndex].orthosigerrsum[radBin] += 
					      osWeight*osWeight*orthodenscont*orthodenscont;
				
					    lensInStruct[lensIndex].sshsum   += wts_ssh*F;
					    lensInStruct[lensIndex].wsum_ssh += wts_ssh;
					    
					  } // Good sig_inv?
				  
				      } // good radBin? 

				  } // Good quadrant?

			      } // radius within min/max?

			  } // loop over sources

		      } // any sources found in this leaf?

		  } // leaf id is in allowed range?

	      } // loop over found leaf ids

	    // Now compute averages if there were any sources for
	    // this lens, i.e. weight > 0

	    if (lensInStruct[lensIndex].weight > 0.0) 
	      {
	  
		ie1 = xmysum/xpysum;
		ie2 = 2.*xysum/xpysum;
		ie = sqrt( ie1*ie1 + ie2*ie2 );
		lensInStruct[lensIndex].ie = ie;

		mm = 3.0/sqrt(lensInStruct[lensIndex].totPairs);

		for(radBin=0; radBin<parStruct->nbin; radBin++)
		  {

		    if (lensInStruct[lensIndex].wsum[radBin] > 0.0) 
		      {

			// Keep track for "good" lenses
			if (ie < max(mm, maxe)) 
			  {
			    sigwsum += lensInStruct[lensIndex].wsum[radBin];
			    sigsum  += lensInStruct[lensIndex].sigma[radBin];
			  }

			inverse_wsum = 1.0/lensInStruct[lensIndex].wsum[radBin];
		  
			lensInStruct[lensIndex].sigma[radBin] *= inverse_wsum;
			lensInStruct[lensIndex].sigmaerr[radBin] = sqrt(inverse_wsum);

		      }
		    if (lensInStruct[lensIndex].owsum[radBin] > 0.0) 
		      {
			inverse_wsum = 1.0/lensInStruct[lensIndex].owsum[radBin];
		  
			lensInStruct[lensIndex].orthosig[radBin] *= inverse_wsum;
			lensInStruct[lensIndex].orthosigerr[radBin] = sqrt(inverse_wsum);
		      }

		  }
	      }

	    if ( (lensIndex % step) == 0 && (lensIndex != 0)) 
	      {
		printf(".");
		if ( (lensIndex % bigStep) == 0)
		  objShearPrintMeans(lensIndex,parStruct->nLensInStruct,sigsum,sigwsum);
		fflush(stdout);
	      }
	     
	  } // central point unmasked?

      } /* loop over lenses */
    
  } catch (SpatialException &x) {
    printf("%s\n",x.what());
  }

  printf("\n\n");
  printf("Found total of %d triangles\n", nFoundTot);
  printf("Used total of %d triangles\n", nFoundUse);
  printf("Used %d source-lens pairs\n", totalPairs);
  return(0);

}

void
objShearPrintMeans(int lensIndex, int nLensInStruct, float sigsum, float sigwsum)
{

  float meanDenscont;
  float meanDenscontErr;

  if (sigwsum > 0) 
    {
      meanDenscont = sigsum/sigwsum;
      meanDenscontErr = sqrt(1.0/sigwsum);
      printf("\nlens = %d/%d   Mean dens. cont. = %f +/- %f\n",
	     lensIndex,nLensInStruct,
	     meanDenscont,meanDenscontErr);		    
    }
  else 
    {
      printf("\nlens = %d/%d   Mean dens. cont. = 0 +/- 0\n",
	     lensIndex,nLensInStruct);		    
    }
}

int
objShearTestQuad(int16& bad12, 
		 int16& bad23, 
		 int16& bad34,
		 int16& bad41, 
		 float64& theta)
{
  
  static const int UNMASKED=1, MASKED=0;
  static const float 
    PI=3.1415927,
    TWOPI=6.2831853,
    PIOVER2=1.5707963,
    THREE_PIOVER2=4.7123890;

  // 1+2 or 3+4 are not masked
  if ( !bad12 || !bad34 ) 
    {

      // keeping both quadrants
      if ( !bad12 && !bad34 )
	{
	  return(UNMASKED);
	}
      
      // only keeping one set of quadrants
      if (!bad12)
	{
	  if (theta >= 0.0 && theta <= PI)
	    return(UNMASKED);
	  else
	    return(MASKED);
	}
      else
	{
	  if (theta >= PI && theta <= TWOPI)
	    return(UNMASKED);
	  else
	    return(MASKED);
	}


    }

  // 2+3 or 4+1 are not masked
  if ( !bad23 || !bad41 ) 
    {

      // keeping both quadrants
      if ( !bad23 && !bad41 )
	{
	  return(UNMASKED);
	}
      
      // only keeping one set of quadrants
      if (!bad23)
	{
	  if (theta >= PIOVER2 && theta <= THREE_PIOVER2)
	    return(UNMASKED);
	  else
	    return(MASKED);
	}
      else
	{
	  if ( (theta >= THREE_PIOVER2 && theta <= TWOPI) ||
	       (theta >= 0.0           && theta <= PIOVER2) )
	    return(UNMASKED);
	  else
	    return(MASKED);
	}

    }


}

