#!/usr/bin/env python2.5

import sqlite3 as sqlite
import sqlite_reader
import cgi
import rccny_util
import sys
import os

form = cgi.FieldStorage()

print "Content-type: text/html\n\n"
print

if not form.has_key("id"):
    print "You must enter a "+contact+" id"
    sys.exit(45)

id=form['id'].value
dbfile=rccny_util.dbfile
table='contacts'

# for redirecting back to entry
url = "contact2html.py?id="+str(id)

# make sure it exists
conn = sqlite.connect(dbfile, isolation_level=None)
conn.text_factory = str
curs = conn.cursor()
query = "select count(*),fullname from "+table+" where id=?"
curs.execute(query, (int(id),))
res = curs.fetchone()

if len(res) == 0:
    print "Contact #'"+str(id)+" not found<br>"
    print "<a href='"+url+"'>Back to contact</a><br>"
    print "<a href='rccny_contacts.py'>Back to "+table+"</a><br>"
    sys.exit(45)

fullname=res[1]
# If confirm=yes is sent then we will delete, else not
if not form.has_key("confirm"):
    print "Are you sure you want to delete the contact ",fullname,"? This cannot be undone."

    crap="""
    <FORM ACTION="some website">
    <SELECT NAME="flavor">
        <OPTION VALUE="van" SELECTED>Vanilla
        <OPTION VALUE="str">Strawberry
        <OPTION VALUE="rr">Rum and Raisin
        <OPTION VALUE="po">Peach and Orange
    </SELECT>
    <BR>
    <INPUT TYPE="submit" VALUE="Make it so!">
    </FORM>
"""




    print "<form action=contact_delete.py>" 
    print "  <input type='hidden' name='id' value='"+str(id)+"'>"
    print "  <input type='submit' name='confirm' value='yes'>"
    print "  <input type='submit' name='confirm' value='no'>"
    #print "  <button type='submit' name='confirm' value='yes'>Yes"
    #print "  <button type='submit' name='confirm' value='no'>&nbsp;&nbsp;No"
    print "</form>"

else:
    val = form['confirm'].value
    if val == 'no':
        print fullname+"' was not deleted<br>"
        print "<a href='"+url+"'>Back to contact '"+fullname+"'</a><br>"
        #print "<a href='rccny_contacts.py'>Back to main page</a><br>"
    elif val == 'yes':
        # build up delete query
        query = "delete from "+table+" where id = ?"
        try:
            curs.execute(query, (id,))
            print "Contact '"+fullname+"' was deleted<br>"

            #print "<a href='rccny_contacts.py'>Back to "+table+"</a><br>"
        except:
            print "Error deleting recipe '"+fullname+"'"
            print "Error info: ",sys.exc_info(),"<br>"
    else:
        pass

print "<p>"
rccny_util.PrintCloseWindow()
conn.close()

