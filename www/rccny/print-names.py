#!/usr/bin/env python2.5

import sqlite_reader
import sys

# Want to print in ncol columns
ncol = 3
colwidth = 35
baseform = '%-'+str(colwidth)+'s '
colorform = '<span style=\"color:green\">%-'+str(colwidth)+'s </span>'
#form = baseform*ncol



def PrintNamesWithColor(keepnames, keepgift):
    print '<html>'
    print '<body>'

    print '<pre>'


    rownames = []
    form=""
    for name,gift in zip(keepnames, keepgift):
        if len(name) > colwidth:
            # Just clip the beginning
            name = name[0:colwidth]
        if gift == 'c':
            thisform = colorform
        else:
            thisform = baseform

        rownames.append(name)
        form = form + thisform

        if len(rownames) == ncol:
            print form % tuple(rownames)
            rownames = []
            form = ''
    
    nleft = len(rownames)
    form = baseform*nleft
    print form % tuple(rownames)


    print '</pre>'
    print '</body>'
    print '</html>'


def PrintNames(keepnames, keepgift, contributors=False):
    rownames = []
    form=baseform*ncol
    for name,gift in zip(keepnames, keepgift):

        if (contributors and gift == 'c') or \
           (not contributors and gift != 'c'):
            if len(name) > colwidth:
                # Just clip the beginning
                name = name[0:colwidth]

            rownames.append(name)

            if len(rownames) == ncol:
                print form % tuple(rownames)
                rownames = []
    
    nleft = len(rownames)
    form = baseform*nleft
    print form % tuple(rownames)



dbfile = 'data/rccny_contacts.db'
reader = sqlite_reader.Reader(dbfile)

query="""
select 
    firstname,lastname,gift 
from 
    contacts 
where 
    lastname != "" 
order by 
    lastname COLLATE NOCASE
"""

datadict = reader.ReadAsDict(query)

keepnames = []
keepgift = []
badnames = ['friends']
for d in datadict:
    first = d['firstname'].strip()
    last = d['lastname']
    if first != '':
        all = last + ', '+first
    else:
        all = last
    all = all.strip()
    if all.lower() not in badnames:
        keepnames.append(all)
        keepgift.append(d['gift'].lower())

#PrintNames(keepnames, keepgift, contributors=False)
PrintNames(keepnames, keepgift, contributors=True)


