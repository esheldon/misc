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


# get the fields
query = 'select * from '+table+' limit 1'
curs.execute(query)
res=curs.fetchone()
if len(res) == 0:
    print "Unknown contact id: ",id

# Get the updated fields
insert_vals = []
alltext = []
skip =['alltext','fullname']
firstname=''
lastname=''
for d in curs.description:

    name = d[0]

    if name not in skip:
        if name == "id":
            val = None 
        else:
            if form.has_key(name):
                #val = str(form[name].value)
                val = unicode(form[name].value,'utf8','ignore')
            else:
                val = '' 

            if name=='firstname':
                firstname=val
            if name=='lastname':
                lastname=val
 
        if val is not None and val != "":
            alltext.append(val)

        insert_vals.append(val)
 

fullname=lastname
if fullname != '' and firstname != '':
    fullname = fullname +', '+firstname
elif firstname != '':
    fullname=firstname

insert_vals.append(fullname)

alltext = " ".join(alltext)
insert_vals.append(alltext)


# prepared statement
qmarks = ['?']*len(insert_vals)
qmarks = ",".join(qmarks)

ps = 'insert into '+table+' values('+qmarks+')'

try:
    curs.execute(ps, insert_vals)
    print "Successfully added contact: ",fullname,"<br>" 

    #print "<a href='rccny_contacts.py'>Back to "+table+"</a><br>"

    curs.execute('select id from contacts order by id desc limit 1')
    data = curs.fetchall()
    id = data[0][0]
    url = "contact2html.py?id="+str(id)
    print "<a href='"+url+"'>View ",fullname,"</a><br>"

except:
    print "Failed to add contact: ",fullname,"<br>"
    print "Error info: ",sys.exc_info(),"<br>"

ru.PrintCloseWindow()
conn.close()


