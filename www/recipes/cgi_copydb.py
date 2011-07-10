#!/usr/bin/env python2.5

import sqlite3
import sqlite_reader
import os

print "Content-type: text/html\n\n"

# The old database file
old_dbfile = 'cathy_recipes.db.bak'

# The new database file
dbfile = '/Users/data/cathy_recipes.db'

if os.path.exists(dbfile):
    os.remove(dbfile)

# build up table def
tdef = ['id integer primary key']
tdef.append('name text')
tdef.append('category text')
tdef.append('notes text')
tdef.append('comments text')
tdef.append('servings text')

tdef.append('item1 text')
tdef.append('item2 text')
tdef.append('item3 text')
tdef.append('item4 text')
tdef.append('item5 text')
tdef.append('item6 text')
tdef.append('item7 text')
tdef.append('item8 text')
tdef.append('item9 text')
tdef.append('item10 text')
tdef.append('item11 text')
tdef.append('item12 text')
tdef.append('item13 text')
tdef.append('item14 text')
tdef.append('item15 text')
tdef.append('item16 text')
tdef.append('item17 text')
tdef.append('item18 text')

tdef.append('step1 text')
tdef.append('step2 text')
tdef.append('step3 text')
tdef.append('step4 text')
tdef.append('step5 text')
tdef.append('step6 text')
tdef.append('step7 text')
tdef.append('step8 text')
tdef.append('step9 text')
tdef.append('step10 text')
tdef.append('step11 text')
tdef.append('step12 text')
tdef.append('step13 text')
tdef.append('step14 text')
tdef.append('step15 text')
tdef.append('step16 text')
tdef.append('step17 text')
tdef.append('step18 text')
tdef.append('alltext text')

ncols = len(tdef)
tdef = ",".join(tdef)

qmarks = ['?']*ncols
qmarks = ",".join(qmarks)
query='insert into recipes values('+qmarks+')'

conn = sqlite3.connect(dbfile,isolation_level=None)
c = conn.cursor()
c.execute('create table recipes ('+tdef+')')

print query,"<br>"

reader = sqlite_reader.Reader(old_dbfile)
res,restup,desc = reader.ReadAsDict("select * from recipes",return_tup=True)

print "Writing to db<br>"
i=0
for row in res:
    alltext = [] 
    for tmp in restup[i]:
        tmp = str(tmp)
        if tmp != "":
            alltext.append( str(tmp) )
    alltext = " ".join(alltext)
    i=i+1
    vals = \
        (row['id'], 
         row['name'],
         row['category'],
         row['notes'],
         row['comments'],
         row['servings'],
         row['item1'],
         row['item2'],
         row['item3'],
         row['item4'],
         row['item5'],
         row['item6'],
         row['item7'],
         row['item8'],
         row['item9'],
         row['item10'],
         row['item11'],
         row['item12'],
         row['item13'],
         row['item14'],
         row['item15'],
         row['item16'],
         row['item17'],
         row['item18'],
         row['step1'],
         row['step2'],
         row['step3'],
         row['step4'],
         row['step5'],
         row['step6'],
         row['step7'],
         row['step8'],
         row['step9'],
         row['step10'],
         row['step11'],
         row['step12'],
         row['step13'],
         row['step14'],
         row['step15'],
         row['step16'],
         row['step17'],
         row['step18'],
         alltext)

    c.execute(query,vals)

conn.close()

conn2 = sqlite3.connect(dbfile)
c2=conn2.cursor()
c2.execute('select id,name from recipes')
for row in c2:
    print row,"<br>"


