#!/usr/bin/env python2.5

import os
from datetime import datetime
import rccny_util as ru

def GetDate():
    date = datetime.today().strftime('%Y-%m-%d-%T')
    return date

def NewName(front):
    date = GetDate()
    name = front+'-'+date+'.db'
    while os.path.exists(name):
        date = GetDate()
        name = front+'-'+date+'.db'

    return name

print "Content-type: text/html\n\n"
print

dir = 'data'
backup_dir = 'backup'
dbfront = os.path.join(dir,'rccny_contacts')
backup_dbfront = os.path.join(backup_dir,'rccny_contacts')

dbfile = dbfront+'.db'
dbfile_backup = NewName(backup_dbfront)

#print dbfile
#print dbfile_backup

comm = 'cp '+dbfile+' '+dbfile_backup
retval = os.system(comm)
if retval != 0:
    print 'Backup command: ',comm,'failed'
else:
    print 'Back up to file: ',dbfile_backup,' was successful'

print '<p>'
ru.PrintCloseWindow()
#print "<a href='rccny_contacts.py'>Back to contact list</a>"

