#!/usr/bin/env python2.5

import sys
import os
import sqlite_reader
import cgi

import rccny_util as ru

import re
pattern = r'\b[A-Za-z0-9._\+%-]+@[A-Za-z0-9._%-]+\.[A-Za-z]{2,4}\b'
pattobj = re.compile(pattern)

nperpage=50

def EmailIsValid(email):
    if pattobj.match(email):
        return True
    else:
        return False

def GetEmails(datadict, bad=False):
    emails = []
    for c in datadict:
        email = c['email']
        email = email.replace(' ','')
        if bad: 
            if not EmailIsValid(email):
                emails.append(email)
        elif EmailIsValid(email):
            emails.append(email)

    return emails



fs = cgi.FieldStorage()
datadict, field, message = ru.GetContactData(fs, flist='email')
print "Content-type: text/plain\n"


bad=False
if fs.has_key('bad'):
    if fs['bad'].value == 'yes':
        bad=True

emails = GetEmails(datadict, bad=bad)
emails.sort()
emails = ',\n'.join(emails)

print emails


