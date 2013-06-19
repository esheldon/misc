import sys
import os
import biggles
import numpy
import esutil as eu

#from optparse import OptionParser
#parser=OptionParser(__doc__)
#parser.add_option("-p","--plist", default=None,
#                  help="which paramts to p")

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

def getres():
    import Tkinter

    root = Tkinter.Tk()

    width = root.winfo_screenwidth()
    height = root.winfo_screenheight() 

    return width,height

def main(fname):
    w,h=getres()
    biggles.configure('default','fontsize_min',0.8)
    biggles.configure("screen","width",w*0.9)
    biggles.configure("screen","height",h*0.9)
    data=read_chain(fname)

    xdata=numpy.arange(data.size)

    npars=data['pars'][0,:].size
    if npars in [6,7,8]:
        nrows=3
        ncols=3
    elif npars in [9,10,11]:
        nrows=4
        ncols=3
    else:
        raise ValueError("support mor npars")
    tab=biggles.Table(nrows, ncols)

    plt=biggles.FramedPlot()
    lnprob = data['lnprob']-data['lnprob'].max()
    plt.add(biggles.Curve(xdata,lnprob) )
    plt.xlabel='step'
    plt.ylabel='ln(prob)'

    tab[0,0] = plt

    npar=data['pars'].shape[1]

    iplt=1
    for i in xrange(npar):
        std=data['pars'][:,i].std()
        binsize=0.2*std

        plt=eu.plotting.bhist(data['pars'][:,i], binsize=binsize,
                              xlabel='par %s' % (i+1),show=False)

        prow = iplt / ncols
        pcol = iplt % ncols
        tab[prow,pcol] = plt
        iplt+=1
    tab.show()

if len(sys.argv) < 2:
    print 'python plot-chain.py filename'
    sys.exit(1)
fname=os.path.expanduser(sys.argv[1])
main(fname)
