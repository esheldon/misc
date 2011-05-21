#!/usr/bin/env python2.5

import cgi
import os
import shutil


print "Content-type: text/html\n\n"
print "Not allowing copying right now<br>"

sys.exit(0)

fname='rccny_contacts.db'
print 'fname=',fname,'<br>'
olddir = 'dataold'
print 'olddir=%s<br>' % olddir
newdir = '/srv/wwwhosts/www.cosmo.bnl.gov/SQLite/esheldon/rccny'
print 'newdir=%s<br>' % newdir

fold = os.path.join(olddir, fname)
fnew = os.path.join(newdir, fname)

print 'copying: %s to %s<br>' % (fold, fnew)


try:
    shutil.copy2(fold, fnew)
    print 'copy success<br>'
except:
    print 'copy failed<br>'

print 'goodbye world 2<br>'
