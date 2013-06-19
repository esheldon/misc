import sys
from sys import stdout


_kern_head="""
#ifndef ONE_OVER_2PI
#define ONE_OVER_2PI 0.15915494309189535
#endif


__kernel void gmix(int nelem, 
                   int nrow,
                   int ncol,  
                   __constant float *image, 
                   __constant float *rows, 
                   __constant float *cols, 
                   global float *output,
                   __constant float *psf_pars,  
                   __constant float *pars_all)
{                                                                     
   int idx = get_global_id(0);
   if (idx >= nelem)
       return;
   int iwalker=idx/(nrow*ncol);
   int pidx=iwalker*6;

   float tmp=0, tmp2=0, T2i=0, pi=0;
   float icc=0, irr=0, irc=0, icc_tot=0, irr_tot=0, irc_tot=0;
   float chi2=0, idet=0, norm=0;
   float tmpi=0, psum_psf=0, ppsf=0;
   int ppidx=0;

   float cenrow=pars_all[pidx+0];
   float cencol=pars_all[pidx+1];
   float e1=pars_all[pidx+2];
   float e2=pars_all[pidx+3];
   float T=pars_all[pidx+4];
   float counts=pars_all[pidx+5];

   float ome1=(1-e1);
   float ope1=(1+e1);
   float row = rows[idx];
   float col = cols[idx];

   int im_idx = row*ncol + col;                 
   float imval = image[im_idx];                 
   float u = row-cenrow;                         
   float v = col-cencol;                        
"""

_kern_tail="""
   tmp *= counts;
   tmp = tmp-imval;             
   output[idx] = -0.5*tmp*tmp;  
} 
"""

_kern_head_old="""
#ifndef ONE_OVER_2PI
#define ONE_OVER_2PI 0.15915494309189535
#endif


__kernel void gmix(int nelem, 
                   int nrow,
                   int ncol,  
                   __constant float *image, 
                   __constant float *rows, 
                   __constant float *cols, 
                   global float *output,
                   __constant float *psf_pars,  
                   float cenrow,
                   float cencol,
                   float e1,
                   float e2,
                   float T,
                   float counts)
{                                                                     
   int idx = get_global_id(0);                                        
   if (idx >= nelem)                            
       return;                                  
   float tmp=0, tmp2=0, T2i=0, pi=0;
   float icc=0, irr=0, irc=0, icc_tot=0, irr_tot=0, irc_tot=0;
   float chi2=0, idet=0, norm=0;                                 
   float tmpi=0, psum_psf=0, ppsf=0;
   int ppidx=0;

   float ome1=(1-e1);
   float ope1=(1+e1);
   float row = rows[idx];
   float col = cols[idx];

   int im_idx = row*ncol + col;                 
   float imval = image[im_idx];                 
   float u = row-cenrow;                         
   float v = col-cencol;                        
"""


def main():
    if len(sys.argv) < 3:
        print 'make-dev-kern.py ngauss ngauss_npsf'
        raise ValueError("halting")

    ngauss=int(sys.argv[1])
    npsf=int(sys.argv[2])

    if ngauss==3:
        # these are actually from turb..oh well
            Fvals = [0.5793612389470884,1.621860687127999,7.019347162356363]
            pvals = [0.596510042804182,0.4034898268889178,1.303069003078001e-07]
    elif ngauss==6:
        Fvals = [0.002467115141477932, 
                 0.018147435573256168, 
                 0.07944063151366336, 
                 0.27137669897479122, 
                 0.79782256866993773, 
                 2.1623306025075739]
        pvals = [0.00061601229677880041, 
                 0.0079461395724623237, 
                 0.053280454055540001, 
                 0.21797364640726541, 
                 0.45496740582554868, 
                 0.26521634184240478]
    elif ngauss==10:
        Fvals = [2.9934935706271918e-07, 
                 3.4651596338231207e-06, 
                 2.4807910570562753e-05, 
                 0.00014307404300535354, 
                 0.000727531692982395, 
                 0.003458246439442726, 
                 0.0160866454407191, 
                 0.077006776775654429, 
                 0.41012562102501476, 
                 2.9812509778548648]

        pvals = [6.5288960012625658e-05, 
                 0.00044199216814302695, 
                 0.0020859587871659754, 
                 0.0075913681418996841, 
                 0.02260266219257237, 
                 0.056532254390212859, 
                 0.11939049233042602, 
                 0.20969545753234975, 
                 0.29254151133139222, 
                 0.28905301416582552]
    else:
        raise ValueError("ngauss 10, 6 or 3")

    print _kern_head
    for F,p in zip(Fvals,pvals): 
        stdout.write("""
        pi= %(pval).16g; 
        T2i = T*%(Fval).16g/2.;
        irr = T2i*ome1;
        irc = T2i*e2;
        icc = T2i*ope1;

        tmp2=0, psum_psf=0;
        """ % {'pval':p, 'Fval':F})

        for ipsf in xrange(npsf):
            text="""
            ppidx = %(ipsf)s*6;
            ppsf=psf_pars[ppidx+0];
            psum_psf += ppsf;

            irr_tot = irr+psf_pars[ppidx+3];
            irc_tot = irc+psf_pars[ppidx+4];
            icc_tot = icc+psf_pars[ppidx+5];

            idet = 1./(irr_tot*icc_tot - irc_tot*irc_tot);
            chi2=icc_tot*u*u + irr_tot*v*v - 2.0*irc_tot*u*v;
            chi2 *= idet;
            norm = ONE_OVER_2PI*sqrt(idet);

            tmp2 += norm*ppsf*exp( -0.5*chi2 );  
            """
            text=text % {'ipsf':ipsf}
            stdout.write(text)

        stdout.write("""
        tmp2 *= pi/psum_psf;
        tmp += tmp2;
        """)

    print _kern_tail

main()
