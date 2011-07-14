#!/usr/bin/env python2.5

import sqlite3

print "Content-type: text/html\n\n"
print "<body bgcolor='#ffffcc'>"

print "<form action='create_newdb.py'>"
print "  Book Name:<br>"
print "  <input type='text' name='name'><br>"
print "  Owner: <br>"
print "  <input type='text' name='owner'><br>"
print "  <input type='submit' value='Submit'>"
print "</form>"
