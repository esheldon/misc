#!/usr/bin/env python2.5

import sqlite3 as sqlite
import cgi
import recipe_util
import sys
import os

form = cgi.FieldStorage()

print "Content-type: text/html\n\n"

if not form.has_key("dbfile"):
    print "You must enter a dbfile"
    sys.exit(45)

dbfile = form['dbfile'].value

recipe=recipe_util.Recipe(dbfile=dbfile)
recipe.create(form=form)

