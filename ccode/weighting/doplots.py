import recfile
import biggles
import esutil as eu

pcolor='blue'
wcolor='red'

mmin=15
mmax=26
def readcat(fname):
    dtype=[('zspec','f8'),
           ('zphot','f8'),
           ('weight','f8'),
           ('mag','f8',5)]

    print fname
    r = recfile.Open(fname, 'r', delim=' ', dtype=dtype)
    data = r[:]
    del r

    eu.numpy_util.ahelp(data)
    print data['mag'][0]
    print data['mag'][-1]

    return data

def histmag_1band(train, photo, band, dokey=False):
    """
    Train should have the weights properly set
    """
    bands = ['u','g','r','i','z']
    bin=0.1

    # histogram of training set mag
    th = eu.stat.histogram(train['mag'][:,band],min=mmin,max=mmax,binsize=bin)
    wdict = eu.stat.histogram(train['mag'][:,band],min=mmin,max=mmax,binsize=bin,
                              weights=train['weight'])
    ph = eu.stat.histogram(photo['mag'][:,band],min=mmin,max=mmax,binsize=bin)

    junk="""
    th = eu.stat.histogram(train['mag'][:,band],binsize=bin)
    wdict = eu.stat.histogram(train['mag'][:,band],binsize=bin,
                              weights=train['weight'])
    ph = eu.stat.histogram(photo['mag'][:,band],binsize=bin)
    """

    th=th/float(th.sum())
    wh=wdict['whist']/float(wdict['whist'].sum())
    ph=ph/float(ph.sum())
    
    p_th = biggles.Histogram(th, x0=mmin, binsize=bin)
    p_wh = biggles.Histogram(wh, x0=mmin, color=wcolor, binsize=bin )
    p_ph = biggles.Histogram(ph, x0=mmin, color=pcolor, binsize=bin)
    plt = biggles.FramedPlot()
    plt.add(p_th,p_wh,p_ph)
    plt.xlabel = bands[band]

    if dokey:
        p_th.label = 'train'
        p_ph.label = 'photo truth'
        p_wh.label = 'weighted train'
        key=biggles.PlotKey(0.6,0.9, [p_th,p_ph,p_wh])
        plt.add(key)
    return plt

def histmag(train, photo):
    #a=biggles.FramedArray(3,2)
    a=biggles.Table(3,2)
    a[0,0] = histmag_1band(train,photo,0)
    a[0,1] = histmag_1band(train,photo,1)
    a[1,0] = histmag_1band(train,photo,2)
    a[1,1] = histmag_1band(train,photo,3)
    a[2,0] = histmag_1band(train,photo,4,dokey=True)

    a.write_eps('data/maghist.eps')

def histz(train, photo):
    # idiot put rows first
    tab = biggles.Table( 2, 1 )

    wh_plt = biggles.FramedPlot()
    wmax = 0.0006
    wbin = wmax/30
    whist = eu.stat.histogram( train['weight'], min=0.0, max=wmax, binsize=wbin )
    pwhist = biggles.Histogram( whist, x0=0.0, binsize=wbin)
    wh_plt.add( pwhist )
    wh_plt.xlabel = 'weights'

    plt = biggles.FramedPlot()

    zmin=0.0
    zmax=2.0
    bin=0.03

    # zspec of training set
    htrain = eu.stat.histogram( train['zspec'], min=zmin, max=zmax, binsize=bin )
    htrain = htrain/float(htrain.sum())
    p_htrain = biggles.Histogram( htrain, x0=zmin, binsize=bin )
    p_htrain.label = 'train'

    # zspec of the objects we are trying to fit
    hphoto = eu.stat.histogram( photo['zspec'], min=zmin, max=zmax, binsize=bin )
    hphoto = hphoto/float(hphoto.sum())
    p_hphoto = biggles.Histogram( hphoto, x0=zmin, binsize=bin, color=pcolor )
    p_hphoto.label = 'photo truth'

    # weighted histogram of z from training set
    wdict = eu.stat.histogram(train['zspec'], 
                              weights=train['weight'],
                              min=zmin, max=zmax, binsize=bin )
    whtrain = wdict['whist']
    whtrain = whtrain/float(whtrain.sum())
    p_whtrain = biggles.Histogram( whtrain, x0=zmin, binsize=bin, color=wcolor )
    p_whtrain.label = 'weighted train'

    pk = biggles.PlotKey(0.6,0.9,[p_htrain,p_hphoto,p_whtrain])

    plt.add( p_htrain )
    plt.add( p_hphoto )
    plt.add( p_whtrain )
    plt.add( pk )

    tab[0,0] = wh_plt
    tab[1,0] = plt
        
    tab.write_eps('data/zhist.eps')

def main():
    photo  = readcat('data/photo.tbl')
    wtrain  = readcat('data/train-weight-100.dat')

    print 'wtrain minw:',wtrain['weight'].min(),'maxw:',wtrain['weight'].max()

    histmag(wtrain, photo)
    histz(wtrain,photo)

if __name__=="__main__":
    main()
