#!/usr/bin/env python2.5

import sys
import cgi
import recipe_util as ru

form = cgi.FieldStorage()

print "Content-type: text/html\n\n"
if not form.has_key('name'):
    print "You must enter a book name"
    sys.exit(45)
if not form.has_key('owner'):
    print "You must enter a book owner"
    sys.exit(45)
name=form['name'].value
owner=form['owner'].value

ru.create_db(name, owner)

