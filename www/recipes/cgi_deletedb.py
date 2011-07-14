#!/usr/bin/env python2.5

import cgi
import os
import sys
import recipe_util

form = cgi.FieldStorage()

print "Content-type: text/html\n\n"

if not form.has_key("dbfile"):
    print "You must enter a dbfile"
    sys.exit(45)

dbfile = form['dbfile'].value


confirm=True
if form.has_key("confirm"):
    cval = form['confirm'].value
    if cval == 'no':
        print "Recipe Book '"+dbfile+"' was not deleted<br>"
        print "<a href='recipe_list.py?dbfile="+dbfile+"'>Back to Recipe List</a><br>"
    else:
        confirm=False

recipe_util.delete_db(dbfile, confirm=confirm)

