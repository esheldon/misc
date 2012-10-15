import sys
import os
import biggles
import numpy
import esutil as eu

def read_chain(fname):
    with open(fname) as fobj:
        l=fobj.readline().split()

        nwalk,nstep_per,npar=int(l[0]),int(l[1]),int(l[2])

        #print nwalk,nstep_per,npar
        dtype=[('accept','i1'),
               ('lnprob','f8'),
               ('pars','f8',(npar,))]
        data=numpy.zeros( nwalk*nstep_per, dtype=dtype )

        for i,line in enumerate(fobj):
            l=line.split()
            data['accept'][i] = int(l[0])
            data['lnprob'][i] = float(l[1])
            for j in xrange(2,2+npar):
                data['pars'][i,j-2] = float(l[j])
        return data
def main(fname):
    data=read_chain(fname)

    xdata=numpy.arange(data.size)

    plt=biggles.FramedPlot()
    lnprob = data['lnprob']-data['lnprob'].max()
    plt.add(biggles.Curve(xdata,lnprob) )
    plt.xlabel='step'
    plt.ylabel='ln(prob)'
    plt.show()

    npar=data['pars'].shape[1]

    for i in xrange(npar):
        std=data['pars'][:,i].std()
        binsize=0.2*std

        eu.plotting.bhist(data['pars'][:,i], binsize=binsize,
                          xlabel='par %s' % (i+1))


fname=os.path.expanduser(sys.argv[1])
main(fname)
