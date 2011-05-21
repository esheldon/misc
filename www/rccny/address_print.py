#!/usr/bin/env python2.5

import sys
import os
import sqlite_reader
import cgi

import rccny_util as ru

fs = cgi.FieldStorage()

datadict, field, message = ru.GetContactData(fs)

print "Content-type: text/plain\n"

print_type='tab'
if fs.has_key('type'):
    if fs['type'].value == 'human':
        print_type = fs['type'].value.lower()

if print_type == 'human':
    for c in datadict:
        ret = ru.PrintAddressHuman(c)
        if ret is not None:
            print
else:
    #elements = ['Name','address1','address2','city','state','zip','country']
    elements = ['Name','address1','address2','city','state','zip']
    #topline = ','.join(elements)
    topline = '\t'.join(elements)
    print topline
    for c in datadict:
        ru.PrintAddressTab(c)

