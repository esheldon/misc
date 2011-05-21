#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"
#include "fstream.h"
#include <time.h>
#include "export.h"
#include "objShear.h"
#include "sigmaCritInv.h"
#include "gcircSurvey.h"
#include "lensStruct.h"
#include <stdlib.h>
#include <vector>
#include <algorithm>
using namespace std;

void objShear(int argc, IDL_VPTR argv[])
{

  //IDL_VPTR lensSrc;
  //IDL_VPTR sourceSrc;
  //IDL_VPTR revIndSrc;
  //IDL_VPTR scinvStructSrc;
  //IDL_VPTR parStructSrc;

  int32 *revInd;
  SCINV_STRUCT scinvStruct;
  PAR_STRUCT parStruct;

  int32 nFound;
  int64 nFoundTot=0, nFoundUse=0, totalPairs=0;
  int64 badBin=0, noneFound=0;
  int32 savedepth;
    
  int32 lensIndex;
  
  // working in leaves
  int32 minLeafId, maxLeafId, leafId, leafBin, nLeafBin;

  float32 H0, omega_m;

  float32 logRmin, logRmax, logBinsize, logRkpc;

  int32 step=500;
  int32 bigStep=10000;

  float32 sigwsum=0, sigsum=0;	      

  ///////////////////////
  // Check arguments 
  ///////////////////////

  if (argc < 5) 
    {
      IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
		  "-Syntax: objShear, lensCat, sourceCat, revInd, scinvStruct, parStruct");
    }

  ////////////////////////////
  // Input from IDL
  ////////////////////////////

  IDL_VPTR lensSrc        = argv[0];
  IDL_VPTR sourceSrc      = argv[1];
  IDL_VPTR revIndSrc      = argv[2];
  IDL_VPTR scinvStructSrc = argv[3];
  IDL_VPTR parStructSrc   = argv[4];

  printf("\n");
  printf("Copying scinvStruct\n");
  copyScinvStruct(scinvStructSrc, &scinvStruct);
  
  printf("Copying parStruct\n");
  copyParStruct(parStructSrc, &parStruct);

  printf("Pointing to revInd\n");
  revInd = (int32 *) revIndSrc->value.arr->data;

  // Initialize objects
  LENS lensum(lensSrc);
  SOURCE scat(sourceSrc);  

  // What type of binning?
  if (parStruct.logbin)
    {
      printf("\nBinning in log\n");
      logRmin = log10(parStruct.rmin);
      logRmax = log10(parStruct.rmax);

      logBinsize = ( logRmax - logRmin )/parStruct.nbin;
    }

  // assuming that source catalog is sorted by leafid
  minLeafId = parStruct.minLeafId;
  maxLeafId = parStruct.maxLeafId;

  H0 = parStruct.h*100.0;
  omega_m = parStruct.omega_m;

  // print something to show we are alive
  printf("\n");
  printf("Using depth   = %d\n", parStruct.depth);
  printf("nlens = %d\n", parStruct.nlens);
  printf("nsource         = %d\n", parStruct.nsource);
  printf("Rmin          = %f\n", parStruct.rmin);
  printf("Rmax          = %f\n", parStruct.rmax);

  printf("minLeafId     = %d\n", minLeafId);
  printf("maxLeafId     = %d\n", maxLeafId);

  if (parStruct.logbin) 
    printf("logbin        = %s\n", "Yes");
  else
    printf("logbin        = %s\n", "No");

  if (parStruct.comoving) 
    printf("comoving      = %s\n", "Yes");
  else
    printf("comoving      = %s\n", "No");

  if (parStruct.interpPhotoz) 
    printf("interpPhotoz  = %s\n", "Yes");
  else
    printf("interpPhotoz  = %s\n", "No");

  printf("nbin          = %d\n", parStruct.nbin);
  printf("binsize       = %f\n", parStruct.binsize);
  printf("H0            = %f\n", H0);
  printf("omega_m   = %f\n", parStruct.omega_m);

  printf("\nEach Dot is %d lenses\n\n",step);
  fflush(stdout);

  try {
    // construct index with given depth and savedepth
    savedepth=2;
    htmInterface htm(parStruct.depth,savedepth);  // generate htm interface
    const SpatialIndex &index = htm.index();

    // Loop over all the lenses

    printf("lens = 0/%d   Mean dens. cont. = 0 +/- 0\n",
	   parStruct.nlens);
    for (lensIndex=0;lensIndex<parStruct.nlens;lensIndex++) 
      {

	int16 bad;
	int16 pixelMaskFlags;
      
	// Copy from input structure to a more
	// manageable one

	//copyLensStruct(lensSrc, lensIndex, lensum);

	// Flags for this lens
	//pixelMaskFlags = lensInStruct[lensIndex].pixelMaskFlags;
	//pixelMaskFlags = 
	//  *(int16 *) getRefFromIDLStruct(lensSrc,lensIndex,"PIXELMASKFLAGS",IDL_TYP_INT);

	pixelMaskFlags = lensum.pixelMaskFlags(lensIndex);

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
	    ra      = lensum.ra(lensIndex);
	    dec     = lensum.dec(lensIndex);
	    clambda = lensum.clambda(lensIndex);
	    ceta    = lensum.ceta(lensIndex);

	    zlens = lensum.z(lensIndex);

	    d = cos( lensum.angMax(lensIndex)*D2R );

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
					scat.clambda(srcUse), scat.ceta(srcUse),
					R, theta);

			    Rkpc = R*lensum.DL(lensIndex);
			    
			    // convert to comoving?
			    if (parStruct.comoving) 
			      {
				comov_fac2 = 1./pow(1+zlens, 2);
				comov_fac4 = 1./pow(1+zlens, 4);
				Rkpc = Rkpc*(1+zlens);
			      }

			    // Within our circular radius?
			    if (Rkpc >= parStruct.rmin && Rkpc <= parStruct.rmax)
			      {

				// Is this source in one of the quadrants that is 
				// useful for this lens?
				theta2 = PIOVER2 - theta;
				if (objShearTestQuad(bad12,bad23,bad34,bad41,theta2))
				  {

				    // What kind of binning?
				    if (parStruct.logbin)
				      {
					logRkpc = log10(Rkpc);
					radBin = (int8) ( (logRkpc-logRmin)/logBinsize );
				      }
				    else 
				      {
					radBin = (int8) ( (Rkpc-parStruct.rmin)/parStruct.binsize );
				      }

				    // valid bin number?
				    if (radBin >= 0 && radBin < parStruct.nbin)
				      {

					// This is actually a bottleneck after the
					// searching
					if (parStruct.interpPhotoz) 
					  {
					    zSource = scat.photoz_z(srcUse);
					    zSourceErr = scat.photoz_zerr(srcUse);
					    sig_inv = sigmaCritInv(zlens, zSource, zSourceErr, 
								   &scinvStruct);

					    //printf("zlens = %f\n", zlens);
					    //printf("zSource = %f\n", zSource);
					    //printf("zSourceErr = %f\n", zSourceErr);
					    //printf("sig_inv = %f\n", sig_inv);

					  }
					else
					  {
					    zSource = scat.photoz_z(srcUse);
					    sig_inv = sigmaCritInv(H0, omega_m, 
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
					    e1prime = -(scat.e1(srcUse)*cos2theta + scat.e2(srcUse)*sin2theta);
					    e2prime =  (scat.e1(srcUse)*sin2theta - scat.e2(srcUse)*cos2theta);



					    // covariance
					    e1e2err = scat.e1e2err(srcUse);
					    e1e2err2 = e1e2err*e1e2err;
					    if (e1e2err < 0) e1e2err2 = -e1e2err2;
				    
					    e1e1err2 = 
					      scat.e1e1err(srcUse)*scat.e1e1err(srcUse);

					    e2e2err2 = 
					      scat.e2e2err(srcUse)*scat.e2e2err(srcUse);


					    // Errors in tangential/ortho
					    etan_err2 = 
					      e1e1err2*cos2theta*cos2theta + e2e2err2*sin2theta*sin2theta - 2.0*cos2theta*sin2theta*e1e2err2; 

					    shear_err2 = 0.25*(etan_err2 + SHAPENOISE2);

					    ortho_err2 = 
					      e1e1err2*sin2theta*sin2theta + e2e2err2*cos2theta*cos2theta - 2.0*cos2theta*sin2theta*e1e2err2; 
				    
					    orthoshear_err2 = 0.25*(ortho_err2 + SHAPENOISE2);
				    
					    // density contrast
					    denscont = e1prime/2.0/sig_inv;
					    densconterr2 = shear_err2/sig_inv2;
				    
					    orthodenscont = e2prime/2.0/sig_inv;
					    orthodensconterr2 = orthoshear_err2/sig_inv2;

					    if (parStruct.comoving) 
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
					    
					    *lensum.totPairs(lensIndex) += 1;
					    *lensum.npair(lensIndex,radBin) +=1;
					    
					    rmax_act = 
					      *lensum.rmax_act(lensIndex,radBin);
					    rmin_act =
					      *lensum.rmin_act(lensIndex,radBin);
					    
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
				
					    *lensum.rmax_act(lensIndex,radBin) = rmax_act;
					    *lensum.rmin_act(lensIndex,radBin) = rmin_act;
					    
					    // these are initally sums, then converted to
					    // means later by dividing by wsum
				
					    *lensum.rsum(lensIndex,radBin)     += Rkpc;
					    
					    *lensum.sigma(lensIndex,radBin)    += sWeight*denscont;
					    *lensum.orthosig(lensIndex,radBin) += osWeight*orthodenscont;

					    *lensum.weight(lensIndex)        += sWeight;
					    *lensum.wsum(lensIndex,radBin)  += sWeight;
					    *lensum.owsum(lensIndex,radBin) += osWeight;
				
					    *lensum.sigerrsum(lensIndex,radBin) += 
					      sWeight*sWeight*denscont*denscont;
					    *lensum.orthosigerrsum(lensIndex,radBin) += 
					      osWeight*osWeight*orthodenscont*orthodenscont;
				
					    *lensum.sshsum(lensIndex)   += wts_ssh*F;
					    *lensum.wsum_ssh(lensIndex) += wts_ssh;
					    
					  } // Good sig_inv?
				  
				      } else // good radBin? 
				      {
					badBin += 1;
				      }
				  } // Good quadrant?

			      } // radius within min/max?

			  } // loop over sources

		      } // any sources found in this leaf?

		  } // leaf id is in allowed range?

	      } // loop over found leaf ids

	    // Now compute averages if there were any sources for
	    // this lens, i.e. weight > 0

	    if (*lensum.weight(lensIndex) > 0.0) 
	      {
	  
		ie1 = xmysum/xpysum;
		ie2 = 2.*xysum/xpysum;
		ie = sqrt( ie1*ie1 + ie2*ie2 );
		*lensum.ie(lensIndex) = ie;

		mm = 3.0/sqrt(*lensum.totPairs(lensIndex));

		for(radBin=0; radBin<parStruct.nbin; radBin++)
		  {

		    if (*lensum.wsum(lensIndex,radBin) > 0.0) 
		      {

			// Keep track for "good" lenses
			if (ie < max(mm, maxe)) 
			  {
			    sigwsum += *lensum.wsum(lensIndex,radBin);
			    sigsum  += *lensum.sigma(lensIndex,radBin);
			  }

			inverse_wsum = 1.0/( *lensum.wsum(lensIndex,radBin) );
		  
			*lensum.sigma(lensIndex,radBin) *= inverse_wsum;
			*lensum.sigmaerr(lensIndex,radBin) = sqrt(inverse_wsum);

		      }
		    if (*lensum.owsum(lensIndex,radBin) > 0.0) 
		      {
			inverse_wsum = 1.0/( *lensum.owsum(lensIndex,radBin) );
		  
			*lensum.orthosig(lensIndex,radBin) *= inverse_wsum;
			*lensum.orthosigerr(lensIndex,radBin) = sqrt(inverse_wsum);
		      }

		  }
	      }

	    if ( (lensIndex % step) == 0 && (lensIndex != 0)) 
	      {
		printf(".");
		if ( (lensIndex % bigStep) == 0)
		  objShearPrintMeans(lensIndex,parStruct.nlens,sigsum,sigwsum);
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
  printf("Found %d bad bin\n", badBin);
  fflush(stdout);
  return;

}

void
objShearPrintMeans(int lensIndex, int nlens, float sigsum, float sigwsum)
{

  float meanDenscont;
  float meanDenscontErr;

  if (sigwsum > 0) 
    {
      meanDenscont = sigsum/sigwsum;
      meanDenscontErr = sqrt(1.0/sigwsum);
      printf("\nlens = %d/%d   Mean dens. cont. = %f +/- %f\n",
	     lensIndex,nlens,
	     meanDenscont,meanDenscontErr);		    
    }
  else 
    {
      printf("\nlens = %d/%d   Mean dens. cont. = 0 +/- 0\n",
	     lensIndex,nlens);		    
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


void copyScinvStruct(IDL_VPTR scinvSrc, SCINV_STRUCT *scinvStruct)
{

  float64 *scinv;

  int32 zserr, zs, zl, ind;

  /* we only need a few things actually, so only copy those */

  /* First the big array */

  scinv = 
    (float64 *) getRefFromIDLStruct(scinvSrc, 0, "MEAN_SCINV", IDL_TYP_DOUBLE);

  ind=0;
  for (zserr=0;zserr<NZSERR;zserr++)
    for (zs=0;zs<NZS;zs++)
      for (zl=0;zl<NZL;zl++) 
	{
	  scinvStruct->scinv[zserr][zs][zl] = scinv[ind];
	  ind +=1;
	}

  /* zl info */
  scinvStruct->zlMin = 
    *(float32 *) getRefFromIDLStruct(scinvSrc, 0, "ZLMIN", IDL_TYP_FLOAT);
  scinvStruct->zlStep = 
    *(float32 *) getRefFromIDLStruct(scinvSrc, 0, "ZLSTEP", IDL_TYP_FLOAT);
  
  /* zs info */
  scinvStruct->zsMin = 
    *(float32 *) getRefFromIDLStruct(scinvSrc, 0, "ZSMIN", IDL_TYP_FLOAT);
  scinvStruct->zsStep = 
    *(float32 *) getRefFromIDLStruct(scinvSrc, 0, "ZSSTEP", IDL_TYP_FLOAT);

  /* zs err info */
  scinvStruct->zsErrMin = 
    *(float32 *) getRefFromIDLStruct(scinvSrc, 0, "ZSERRMIN", IDL_TYP_FLOAT);
  scinvStruct->zsErrStep = 
    *(float32 *) getRefFromIDLStruct(scinvSrc, 0, "ZSERRSTEP", IDL_TYP_FLOAT);

}


void copyParStruct(IDL_VPTR parSrc, PAR_STRUCT *parStruct)
{

  parStruct->h = 
    *(float32 *) getRefFromIDLStruct(parSrc, 0, "H", IDL_TYP_FLOAT);

  parStruct->omega_m = 
    *(float32 *) getRefFromIDLStruct(parSrc, 0, "OMEGA_M", IDL_TYP_FLOAT);
  
  parStruct->interpPhotoz = 
    *(int16 *) getRefFromIDLStruct(parSrc, 0, "INTERPPHOTOZ", IDL_TYP_INT);

  parStruct->logbin = 
    *(int16 *) getRefFromIDLStruct(parSrc, 0, "LOGBIN", IDL_TYP_INT);
  parStruct->nbin = 
    *(int16 *) getRefFromIDLStruct(parSrc, 0, "NBIN", IDL_TYP_INT);
  parStruct->binsize = 
    *(float32 *) getRefFromIDLStruct(parSrc, 0, "BINSIZE", IDL_TYP_FLOAT);


  parStruct->rmin = 
    *(float32 *) getRefFromIDLStruct(parSrc, 0, "RMIN", IDL_TYP_FLOAT);
  parStruct->rmax = 
    *(float32 *) getRefFromIDLStruct(parSrc, 0, "RMAX", IDL_TYP_FLOAT);

  parStruct->comoving = 
    *(int16 *) getRefFromIDLStruct(parSrc, 0, "COMOVING", IDL_TYP_INT);

  parStruct->nlens = 
    *(int32 *) getRefFromIDLStruct(parSrc, 0, "NLENS", IDL_TYP_LONG);
  parStruct->nsource = 
    *(int32 *) getRefFromIDLStruct(parSrc, 0, "NSOURCE", IDL_TYP_LONG);

  parStruct->depth = 
    *(int16 *) getRefFromIDLStruct(parSrc, 0, "DEPTH", IDL_TYP_INT);
  parStruct->minLeafId = 
    *(int32 *) getRefFromIDLStruct(parSrc, 0, "MINLEAFID", IDL_TYP_LONG);
  parStruct->maxLeafId = 
    *(int32 *) getRefFromIDLStruct(parSrc, 0, "MAXLEAFID", IDL_TYP_LONG);


}

