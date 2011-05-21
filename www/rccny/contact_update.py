#!/usr/bin/env python2.5

import sqlite3 as sqlite
import cgi
import rccny_util as ru
import sys
import os


form = cgi.FieldStorage()
dbfile = ru.dbfile

# set isolation level so that the database update will work
conn = sqlite.connect(dbfile, isolation_level=None)
conn.text_factory = str
curs = conn.cursor()
table='contacts'

print "Content-type: text/html\n\n"
print

dbfile = 'data/rccny_contacts.db'
if not os.path.exists(dbfile):
    print "dbfile not found: ",dbfile
    sys.exit(45)

if not form.has_key("id"):
    print "You must enter the contact id for updating"
    sys.exit(45)

id=form['id'].value

query = 'select * from '+table+' where id=?'
curs.execute(query,(id,))
res=curs.fetchone()
if len(res) == 0:
    print "Unknown contact id: ",id

# Get the updated fields
insert_vals = []
string_vals = []
setpairs = []
skip =['id','entered','modified','alltext','fullname']
firstname=''
lastname=''
for d in curs.description:
    name = d[0]

    if name not in skip:
        if form.has_key(name):
            # This assumes all fields are text except id
            val = str(form[name].value)
            setpairs.append(name+'=?')
            insert_vals.append(val)
            string_vals.append(val)

            if name=='firstname':
                firstname=val
            if name=='lastname':
                lastname=val
        else:
            val = ''
            setpairs.append(name+'=?')
            insert_vals.append(val)

fullname=lastname
if fullname != '' and firstname != '':
    fullname = fullname +', '+firstname
elif firstname != '':
    fullname=firstname

insert_vals.append(fullname)
setpairs.append('fullname=?')

alltext = " ".join(string_vals)
insert_vals.append(alltext)
setpairs.append('alltext=?')

# set up the ?=? pairs
setpairs = ",".join(setpairs)

# prepared statement
ps = "update "+table+" set "+setpairs+" where id = ?"
insert_vals.append(id)


try:
    curs.execute(ps, insert_vals)
    print "Successfully updated contact: ",fullname,"<br>" 

    url = "contact2html.py?id="+str(id)
    print "<a href='"+url+"'>View ",fullname,"</a><br>"

except:
    print "Failed to update contact: ",fullname,"<br>"
    print "Error info: ",sys.exc_info(),"<br>"

ru.PrintCloseWindow()
conn.close()


