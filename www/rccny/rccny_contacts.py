#!/usr/bin/env python2.5

import sys
import os
import sqlite_reader
import cgi

import rccny_util as ru

import urllib

print "Content-type: text/html\n\n"
print


qs = os.environ['QUERY_STRING']

# Get inputs
fs = cgi.FieldStorage()

# Get the required columns for the main table
flist = \
    'id,firstname,lastname,entered,modified,gift,origin,organization,contact,performer,status,email,salutation'
datadict,field,message = ru.GetContactData(fs, flist=flist)

#
# Top header
#

name="RCCNY Contact List"
ru.PrintHead(name)

print "<table width='100%' cellpadding=0 border=0>"
print "  <tr>"
print "    <td><font size=6><a href='rccny_contacts.py'>",name,"</a></font></td>"

codecall = "<a href='./Codes.htm' target='_blank'>Codes</a>"

pecall = "<a href='email_print.py"
if qs != '':
    pecall = pecall + '?'+qs
pecall = pecall + "' target='_blank'>Print Emails</a>"

pacall = "<a href='address_print.py"
if qs != '':
    pacall = pacall + '?'+qs
pacall = pacall + "' target='_blank'>Print Addresses</a>"

pacall += " (<a href='address_print.py"
if qs != '':
    pacall = pacall + '?'+qs
pacall = pacall + "&type=human' target='_blank'><em>readable</em></a>)"


bcall = "<a href='backup.py' target=_blank>Backup</a>"
ncall = "<a href='contact_form.py' target=_blank>Add a new contact</a>"

calls = pecall + ' | ' + pacall+' | '+codecall+' | '+bcall+' | '+ncall
print "    <td align='right'>",calls,"</td>"
print "  </tr>"
print "</table>"

# Search form
ru.PrintSearchForm(fs, field=field)





# Link to get contributors
q=urllib.quote_plus("select * from contacts where gift like 'c' and contact='yes'")
print "Click <a href='rccny_contacts.py?query="+q+"'>here</a> for a list of Contactable Contributors"

if fs.has_key('page'):
    pagestr = " value='"+fs['page'].value+"'"
else:
    pagestr=''

pattern = r'\b[A-Za-z0-9._\+%-]+@[A-Za-z0-9._%-]+\.[A-Za-z]{2,4}\b'
print "<br>"
print "<form id='noborder' action=rccny_contacts.py>"
print "    Click for a list of email contacts (page # optional)"
print "    <input type='hidden' name='query' value='select * from contacts where (contact NOT LIKE \"dz\" and contact NOT LIKE \"w\" AND contact NOT LIKE \"z\") AND status NOT LIKE \"c\" AND email REGEXP \""+pattern+"\" ORDER BY email'>"
print "    <input size=5 type='text' name='page' "+pagestr+">"
print "    <input type='submit' value='Submit'>"
print "</form>"


q=urllib.quote_plus("SELECT * FROM contacts WHERE (contact NOT REGEXP '.*z.*' and contact NOT LIKE 'w' AND contact NOT LIKE 'u') AND status NOT LIKE 'c' AND fullname != 'None' AND fullname NOT LIKE '%@%' AND fullname != '' ORDER BY contact")
q2 = urllib.quote_plus("select * from contacts where email = '' and gift <> '' and contact = 'yes' order by lastname")


print "<br>"
print "Click <a href='rccny_contacts.py?query="+q+"'>here</a> for a list of snail mail contacts or <a href='rccny_contacts.py?query="+q2+"'>here</a> for contacts with no email"


q3 = """
SELECT 
    * 
FROM 
    contacts 
WHERE 
    contact NOT REGEXP '.*z.*' 
    AND contact NOT LIKE 'w' 
    AND contact NOT LIKE 'u' 
    AND gift NOT LIKE 'c' 
    AND status NOT LIKE 'C' 
    AND contact NOT LIKE 'A' 
    AND status not like 'A' 
    AND status NOT LIKE 'c' 
    AND fullname != 'None' 
    AND fullname NOT LIKE '%@%' 
    AND fullname != '' 
    AND city != ''
    AND state != ''
    AND zip != ''
"""


q3rand = q3 + " order by random() limit 1000"
q3rand=urllib.quote_plus(q3rand)
q3 = urllib.quote_plus(q3)
print "<br>"
print "Click <a href='rccny_contacts.py?query="+q3+"'>here</a> for a list of snail mail contacts for fundraising"
print "<br>"
print "Click <a href='rccny_contacts.py?query="+q3rand+"'>here</a> for a random 1000 of the above"


#q=urllib.quote_plus('select * from contacts where contact like "a" and status not like "c"  and fullname != "None" and fullname not like "%@%" and fullname != ""')
#print "<br>"
#print "Click <a href='rccny_contacts.py?query="+q+"'>here</a> for the ones we missed"

# Main table
ru.PrintTableByName(datadict, fs, caption=message)


