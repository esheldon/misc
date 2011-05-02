from __future__ import print_function
import os
import numpy

def output_dtype():
    return [('x','f8',3),
            ('p','f8',3)]

def read_output(filename):
    fobj = open(filename, 'r')

    data={}

    data['nstep']      = numpy.fromfile(fobj, dtype='i8', count=1)[0]
    print("Found nstep:",data['nstep'])
    data['nparticles'] = numpy.fromfile(fobj, dtype='i8', count=1)[0]
    print("Found nparticles:",data['nparticles'])
    data['xmax']       = numpy.fromfile(fobj, dtype='f8', count=1)[0]
    print("Found xmax:",data['xmax'])
    data['tstep']      = numpy.fromfile(fobj, dtype='f8', count=1)[0]
    print("Found tstep:",data['tstep'])
    data['xsoft']      = numpy.fromfile(fobj, dtype='f8', count=1)[0]
    print("Found xsoft:",data['xsoft'])

    steps = []
    dt = output_dtype()
    for i in xrange(data['nstep']):
        stepid = numpy.fromfile(fobj, dtype='i8', count=1)[0]
        if stepid != i:
            raise ValueError("Expected step id to be %i but got %i" % (i,stepid))

        particles = numpy.fromfile(fobj, dtype=dt, count=data['nparticles'])
        steps.append(particles)

    data['steps'] = steps
    return data

def plot_steps(filename):
    import biggles
    data = read_output(filename)
    
    d=os.path.dirname(filename)

    for i in xrange(data['nstep']):
        #epsfile = filename.replace('.dat', '-%06i.eps' % i)
        pngfile = filename.replace('.dat', '-%06i.png' % i)

        plt=biggles.FramedPlot()
        p = biggles.Points(data['steps'][i]['x'][:,0], 
                           data['steps'][i]['x'][:,1], 
                           symboltype='filled circle', symbolsize=0.8)

        plt.add(p)
        plt.xrange=[0,data['xmax']]
        plt.yrange=[0,data['xmax']]
        plt.xlabel = 'x'
        plt.ylabel = 'y'
        plt.title = 'step %i' % i

        print("Writing image:",pngfile)
        #plt.write_eps(epsfile)
        plt.write_img(800,800,pngfile)

