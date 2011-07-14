#!/usr/bin/env python2.5
"""
Write an html file for the input recipe id
cgi_recipi2html.py?id=35&dbfile=recipe_database.db
"""

import cgi
import recipe_util as ru

print "Content-type: text/html\n\n"

form = cgi.FieldStorage()

if not form.has_key("id") or not form.has_key("dbfile"):
    print "You must send an id and dbfile to this script"
else:
    doprint=False
    if form.has_key("print"):
        if form['print'].value == "yes":
            doprint=True
    recipe=ru.Recipe(form['dbfile'].value, form['id'].value)
    recipe.load()
    recipe.tohtml(doprint=doprint)

