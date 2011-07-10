#!/usr/bin/env python2.5

import sqlite3 as sqlite
import sqlite_reader
import cgi
import recipe_util
import sys
import os

form = cgi.FieldStorage()

print "Content-type: text/html\n\n"

if not form.has_key("dbfile"):
    print "You must enter a dbfile"
    sys.exit(45)

if not form.has_key("id"):
    print "You must enter a recipe id"
    sys.exit(45)

dbfile=form['dbfile'].value
id=form['id'].value

recipe=recipe_util.Recipe(dbfile=dbfile, id=id)

# by default we will ask for confirmation
confirm=True
if form.has_key('confirm'):
    cval=form['confirm'].value
    if cval == 'no':
        # Do not delete!
        recipe.load()
        name=recipe.recipe['name']
        print "Recipe '%s' was not deleted<br>" % name
        url = "cgi_recipe2html.py?id=%s&dbfile=%s" % (id, dbfile)
        print "<a href='"+url+"'>Back to recipe '%s'</a><br>" % name
        print "<a href='recipe_list.py?dbfile=%s'>Back to main page</a><br>" % dbfile
    else:
        # we will delete
        confirm=False


recipe.delete(confirm=confirm)


