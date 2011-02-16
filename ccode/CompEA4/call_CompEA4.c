#include <stdlib.h>
#include <stdio.h>
#include "export.h"
#include "CompEA4.h"

int call_CompEA4(int argc, void *argv[])
{

  double *m_rr_cc, *m_rr_cc_psf, *e1, *e2, *e1_psf, *e2_psf, 
    *rho4, *rho4_psf, *e1_out, *e2_out, *R_out;
  int *tNdata, Ndata, *flags;

  double Tratio, a4, a4_psf, 
    e1_send, e2_send, 
    e1p_send, e2p_send, e1_ret, e2_ret, R_ret;

  int i;

  m_rr_cc = (double *) argv[0];
  m_rr_cc_psf = (double *) argv[1];
  e1 = (double *) argv[2];
  e2 = (double *) argv[3];
  e1_psf = (double *) argv[4];
  e2_psf = (double *) argv[5];
  rho4 = (double *) argv[6];
  rho4_psf = (double *) argv[7];
  tNdata = (int *) argv[8];

  e1_out = (double *) argv[9];
  e2_out = (double *) argv[10];
  R_out  = (double *) argv[11];
  flags  = (int *) argv[12];

  Ndata = *tNdata;

  for(i=0;i<Ndata;++i)
    {

      Tratio = m_rr_cc_psf[i]/m_rr_cc[i];

      a4 = rho4[i]/2. - 1.;
      a4_psf = rho4_psf[i]/2. - 1.;
      
      flags[i] = CompEA4(Tratio, 
			 e1_psf[i], e2_psf[i], a4_psf, 
			 e1[i], e2[i], a4, 
			 &e1_ret, &e2_ret, &R_ret);

      e1_out[i] = e1_ret;
      e2_out[i] = e2_ret;
      R_out[i]  = 1. - R_ret;

    }

}

      /*
	Tratio = ( *(m_rr_cc_psf + i) )/( *(m_rr_cc + i ) );
	a4 = ( *(rho4+i) )/2. - 1.;
	a4_psf = ( *(rho4_psf+i) )/2. - 1.;

	e1_send = *(e1 + i);
	e2_send = *(e2 + i);
	e1p_send = *(e1_psf + i);
	e2p_send = *(e2_psf + i);
	
	*(flags +i) = CompEA4(Tratio, 
			    e1p_send, e2p_send, a4_psf, 
			    e1_send, e2_send, a4, 
			    &e1_ret, &e2_ret, &R_ret);

        *(e1_out + i) = e1_ret;
        *(e2_out + i) = e2_ret;
        *(R_out + i)  = 1. - R_ret;
      */
