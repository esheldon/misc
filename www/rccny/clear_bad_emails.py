#!/usr/bin/env python2.5
"""
    Read a list of returned emails and set email='' for these in the database.
    Also set a comment explaining what happened.
    
    TODO: Forgot to update the fulltext field.
"""

import sys
import sqlite_reader
import sqlite3 as sqlite
import cgi

import rccny_util as ru

fs = cgi.FieldStorage()
datadict, field, message = ru.GetContactData(fs, flist='id,email,comments')
print "Content-type: text/html\n\n"

# Read the bad emails
file = open('data/returned-emails.dat','r')
bad_emails = []
for line in file:
    email = line.strip().lower()
    bad_emails.append(email)
    #print email,'<br>'

# compare to emails from the db
matched_contacts = []
matched_emails = []
for c in datadict:
    em = c['email'].strip().lower()
    em = em.replace(' ','')
    if em in bad_emails:
        matched_contacts.append(c)
        matched_emails.append(em)

# Sort list of dictionaries: python >= 2.4
from operator import itemgetter
matched_contacts = sorted(matched_contacts, key=itemgetter('email'))

print '<table border=1>'
for c in matched_contacts:
    print '<tr><td>',c['id'],'</td><td>',c['email'],'</td></tr>'
print '</table>'
print 'Number of bad emails:',len(matched_contacts),'<br>'

# print ones that did not match
for em in bad_emails:
    if em not in matched_emails:
        print 'Unmatched email:', em,'<br>'


if len(matched_contacts) > 0:
    # set isolation level so that the database update will work
    dbfile = ru.dbfile
    conn = sqlite.connect(dbfile, isolation_level=None)
    conn.text_factory = str
    curs = conn.cursor()
    table='contacts'

    # Prepare a query
    query = \
        ' UPDATE '+ \
            table+\
        ' SET '+\
        '   email=?, comments=? '+\
        ' WHERE '+\
        '   id=?'

    print '<br>',query,'<br><br>'
    for c in matched_contacts:
        id = c['id']
        comments = c['comments']
        comments = \
            comments+' Email bounced: '+c['email']+'; deleted email.'
        print id,comments,'<br>'
        curs.execute(query,('',comments,id,))

    conn.close()
