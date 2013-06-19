from __future__ import print_function
import os
import numpy
import sys

from esutil import recfile

def output_dtype():
    return [('step','i8'),
            ('id','i8'),
            ('x','f8',3),
            ('p','f8',3)]

def read_output(filename):
    fobj = open(filename, 'r')
    data={}

    data['nstep'] = long(fobj.readline().split()[1])
    print("Found nstep:",data['nstep'])

    data['nparticles'] = long(fobj.readline().split()[1])
    print("Found nparticles:",data['nparticles'])

    ls=fobj.readline().split()[1:]
    data['xmax'] = [float(x) for x in ls]
    print("Found xmax:",data['xmax'])

    data['tstep']      = float(fobj.readline().split()[1])
    print("Found tstep:",data['tstep'])
    fobj.close()

    dt=output_dtype()
    robj = recfile.Recfile(filename,'r',dtype=dt,delim=' ',
                           nrows=data['nstep']*data['nparticles'],
                           skiplines=4)

    data['particles'] = robj.read()
    return data

def plot_steps(filename):
    import biggles

    print("reading",filename)
    data = read_output(filename)
    
    d=os.path.dirname(filename)

    p = data['particles']
    n = data['nparticles']
    for i in xrange(data['nstep']):
        #epsfile = filename.replace('.dat', '-%06i.eps' % i)
        pngfile = filename.replace('.dat', '-%06i.png' % i)

        plt=biggles.FramedPlot()
        
        psub = p[i*n:(i+1)*n]
        pp = biggles.Points(psub['x'][:,0], 
                            psub['x'][:,1], 
                            symboltype='filled circle', symbolsize=0.8)

        plt.add(pp)
        plt.xrange=[0,data['xmax'][0]]
        plt.yrange=[0,data['xmax'][0]]
        plt.xlabel = 'x'
        plt.ylabel = 'y'
        plt.title = 'step %i' % i
        
        print("Writing image:",pngfile)
        plt.write_img(800,800,pngfile)

def main():
    if len(sys.argv) < 2:
        print("usage: python nb.py filename")
        sys.exit(1)
    
    filename=sys.argv[1]
    plot_steps(filename)

main()
