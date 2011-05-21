#!/usr/bin/env python2.5

import sqlite3 as sqlite
import sys
import getopt
import rccny_util as ru

def FixBadCol(lsp, coldict):
    fncol = coldict['firstname']
    lncol = coldict['lastname']
    emailcol = coldict['email']
    ccol = coldict['contact']

    ec = coldict['entered']
    mc = coldict['modified']

    if lsp[fncol].strip() == 'Ilse':
        lsp[emailcol] = 'Ilse.de.Veer@mercer.com'

    lsp[fncol] = lsp[fncol].replace('"','')
    lsp[lncol] = lsp[lncol].replace('"','')

    # Don't allow multiple email addresses
    em = lsp[emailcol]
    ems = em.split(';')
    email = ems[0]
    lsp[emailcol] = email.replace('"','')

    # Fix date format
    if lsp[ec].strip() == '':
        lsp[ec] = '01/01/85'
    lsp[ec] = ru.ConvertDate(lsp[ec])
    lsp[mc] = ru.ConvertDate(lsp[mc])


    if lsp[ccol] == '':
        lsp[ccol] = 'yes'

    # There are some bad characters
    nl = []
    for l in lsp:
        val = unicode(l,'ascii','replace') 
        val = val.encode('ascii','replace')
        val = val.replace('?','')
        val = val.strip()
        nl.append(val) 

    return nl
      

print "Content-type: text/html\n\n"
print

# Main

# User must input the tabbed file and the output db file
#file = sys.argv[1]
#dbfile = sys.argv[2]

file='data/rccny_contacts.txt'
dbfile='data/rccny_contacts.db.test'

table = 'contacts'
metatable = 'meta'

# First read the first line and use as the field name definition
fp = open(file,'r')
line = fp.readline()
vals = line.split('\t')

#
# build up table def from the column names on the first line
#
coldict = {}
coln=0
ttdef = ['id integer primary key']

for v in vals:
    v = v.strip()
    v = v.replace(' ','')
    v = v.replace('-','')
    v = v.replace('/','And')
    if v == 'FirstNameAndMI':
        v='FirstName'
    if v == 'No':
        v = 'Contact'

    v = v.lower()
    coldict[v] = coln


    ttdef.append(v+' text')
    coln=coln+1

ttdef.append('fullname text')
ttdef.append('alltext text')
print ttdef
print '<p>'


ncols = len(ttdef)
tdef = ",".join(ttdef)

print tdef
print '<p>'

# The insert query
qmarks = ['?']*ncols
qmarks = ",".join(qmarks)
query='insert into '+table+' values('+qmarks+')'

# make connection and create table
conn = sqlite.connect(dbfile,isolation_level=None)
conn.text_factory = str
c = conn.cursor()
c.execute('create table '+table+' ('+tdef+')')

# now insert each row
id=0
for line in fp:
    
    # split the line by tabs
    lsp = line.split('\t')
    lsp = FixBadCol(lsp, coldict)
    i = 0
    alltext = []
    for l in lsp:

        if lsp[i] != '':
            alltext.append(lsp[i])
        i = i+1

    # remove newline at end
    if lsp[-1] == '\r\n':
        lsp[-1] = ''

    fullname = ru.GetFullNameLSP(lsp, coldict)
    vals = [id]
    vals.extend(lsp)

    if fullname != '':
        alltext.append(fullname)
    alltext = " ".join(alltext)
    vals.append(fullname)
    vals.append(alltext)

    c.execute(query,vals)
    id = id+1

#"""
print 'Creating indexes'
c.execute('create index rccny_status_index on '+table+' (status)')
c.execute('create index rccny_performer_index on '+table+' (performer)')
c.execute('create index rccny_alltext_index on '+table+' (alltext)')
c.execute('create index rccny_fullname_index on '+table+' (fullname)')
c.execute('create index rccny_email_index on '+table+' (email)')
c.execute('create index rccny_contact_index on '+table+' (contact)')
#"""

print '<p>'
print 'Creating triggers'
# trigger setting the entered date on insert
query = """
CREATE TRIGGER insert_contacts_entered AFTER INSERT ON contacts 
BEGIN 
  UPDATE contacts SET entered = DATE('NOW') 
     WHERE rowid=new.rowid; 
END"""
c.execute(query)

# trigger setting the modified date on update
query = """
CREATE TRIGGER update_contacts_modified AFTER UPDATE ON contacts 
BEGIN 
  UPDATE contacts SET modified = DATE('NOW') 
     WHERE rowid=new.rowid; 
END"""
c.execute(query)


conn.close()

conn2 = sqlite.connect(dbfile)
c2=conn2.cursor()
c2.execute('select id,LastName from '+table+' limit 10')
print '<p>'
print 'Printing first 10<br>'
print '-------------------------------------------------<br>'
for row in c2:
    print row,'<br>'




