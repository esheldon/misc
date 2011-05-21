#!/usr/bin/env python2.5

import sqlite_reader
import os
import re

import cgi

import rccny_util as ru

reg = {}
reg['item'] = re.compile('.*item.*')
reg['step'] = re.compile('.*step.*')

dbfile='data/rccny_contacts.db'
table='contacts'


def Contact2html(id, doprint=False):

    if not os.path.exists(dbfile):
        print "Requested dbfile does not exist: ",dbfile
        return

    # Query the db
    reader = sqlite_reader.Reader(dbfile)

    datadict,datatup,desc = \
            reader.ReadAsDict("select * from "+table+" where id = "+str(id),
                              return_tup=True)
    reader.close()

    if len(datadict) == 0:
        print "No such id found: ",id
        return

    # Simplify notation
    datatup = datatup[0]
    datadict = datadict[0]

    ru.PrintHead(datadict['fullname'])
    WriteMainTable(id, datadict, datatup, desc, doprint=doprint)

    if not doprint:
        ru.PrintCloseWindow()

def PrintPhone(datadict):
    
    ph1 = datadict['tel1']
    ext1 = datadict['ext1']
    ph2 = datadict['tel2']
    ext2 = datadict['ext2']
    cell = datadict['cellular']
    fax1 = datadict['fax1']
    fax2 = datadict['fax2']

    # First phone numbers
    phone = []
    if ph1 != '':
        if ext1 != '':
            ph1 = ph1 + ' ext: '+ext1
        phone.append(ph1)
    if ph2 != '':
        if ext2 != '':
            ph2 = ph2 + ' ext: '+ext2
        phone.append(ph2)

    if cell != '':
        cell = 'cell: '+cell
        phone.append(cell)

    if len(phone) != 0:
        phone = '<br>'.join(phone)
        print "        <tr><td><em>Phone:</em></td><td>",phone,"</td></tr>"

    # Now fax numbers
    fax = []
    if fax1 != '':
        fax.append(fax1)
    if fax2 != '':
        fax.append(fax2)

    if len(fax) != 0:
        fax = '<br>'.join(fax)
        print "        <tr><td><em>Fax::</em></td><td>",fax,"</td></tr>"

def PrintOrg(datadict):
    org = []
    org1 = datadict['organization']
    org2 = datadict['organization2']
    if org1 != '':
        org.append(org1)
    if org2 != '':
        org.append(org2)
    if len(org) > 0:
        org = '<br>'.join(org)
        print "        <tr><td><em>Organization:</em></td><td>",org,"</td></tr>"

def GetWebpageURL(webpage):
    if webpage != '':
        if webpage.find('http://') == 0:
            wurl = webpage
        else:
            wurl = 'http://'+webpage
    else:
        wurl = ''

    if wurl != '':
        wurl = "<a href='"+wurl+"' target='_blank'>"+webpage+"</a>"
    return wurl


def WriteMainTable(id, datadict, datatup, desc, doprint=False):

    dbname = 'RCCNY Contacts'
    if doprint:
        tclass='contact_print'
    else:
        tclass='contact'

    border = "0"
    name=datadict['fullname']
    title = datadict['title']
    status=datadict['status']
    perf=datadict['performer']

    email = datadict['email']
    eurl = "<a href='mailto:"+email+"'>"+email+"</a>"
    webpage = datadict['web']
    wurl = GetWebpageURL(webpage)

    orig = datadict['origin']
    ccode = datadict['contact']
    entered = datadict['entered']
    modified = datadict['modified']

    gift = datadict['gift']

    comm = datadict['comments']

    codecall = "<a href='./Codes.htm' target='_blank'>Codes</a>"
    ucall = "contact_form.py?id="+str(id)
    ucall = "<a href='"+ucall+"'>Edit</a>"
    dcall = "contact_delete.py?id="+str(id)+"&name="+name
    dcall = "<a href='"+dcall+"'>Delete</a>"
    pcall = "<a href='contact2html.py?id="+str(id)+"&print=yes' target='_blank'>Printer Friendly Version</a>"

    print "<table>"
    #print "   <tr><td align='center'><font size=5>Recipe</font></td></tr>"
    mainurl = "<a href='rccny_contacts.py'>"+dbname+"</a>"
    editurl = ucall+" | "+dcall + ' | '+codecall+" | "+pcall
    # A Header with recipe book name, edit, delete, printable
    if not doprint:
        print  "   <tr><td width='100%'>"
        print "         <table width='100%' border="+border+">"
        print "           <tr><td align='left'><font size=5>"+mainurl+"</font></td><td align='right'>"+editurl+"</td></tr>"
        print "         </table>"
        print "   </td></tr>"

    print "   <tr><td width='100%'>"
    print "      <table class='"+tclass+"' border="+border+">"
    print "        <tr><td><em>Name:</em></td><td><b>",name,"</b></td></tr>"

    if title != '':
        print "        <tr><td><em>Title:</em></td><td>",title,"</td></tr>"
    PrintOrg(datadict)
    if email != '':
        print "        <tr><td><em>Email:</em></td><td>",eurl,"</td></tr>"
    ru.PrintAddressTD(datadict)
    PrintPhone(datadict)
    if wurl != '':
        print "        <tr><td><em>Webpage:</em></td><td>",wurl,"</td></tr>"

    if perf != '':
        print "        <tr><td><em>Performer:</em></td><td>",perf,"</td></tr>"

    if status != '':
        print "        <tr><td><em>PStatus Code:</em></td><td>",status,"</td></tr>"

    if gift != '':
        print "        <tr><td><em>Gift Code:</em></td><td>",gift,"</td></tr>"

    if orig != '':
        print "        <tr><td><em>Origin Code:</em></td><td>",orig,"</td></tr>"
    print "        <tr><td><em>Contact:</em></td><td>",ccode,"</td></tr>"
    if comm != '':
        print "        <tr><td><em>Comments:</em></td><td>",comm,"</td></tr>"
    if entered != '':
        print "        <tr><td><em>Date Entered:</em></td><td>",entered,"</td></tr>"
    if modified != '':
        print "        <tr><td><em>Date Modified:</em></td><td>",modified,"</td></tr>"

    print "        <tr><td>&nbsp;</td><td width=500>&nbsp;</td></tr>"


    print "     </table>" 
    print "  </td></tr>"
    print "</table>"



if __name__=="__main__":
    """
    Write an html file for the person's id
    row2html.py?id=35
    """

    print "Content-type: text/html\n\n"
    print

    form = cgi.FieldStorage()

    if not form.has_key("id"):
        print "You must send an id to row2html"
    else:
        doprint=False
        if form.has_key("print"):
            if form['print'].value == "yes":
                doprint=True
        Contact2html(form['id'].value, doprint=doprint)

