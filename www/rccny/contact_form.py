#!/usr/bin/env python2.5

import sqlite_reader
import os
import re

import cgi

# this is a list to preserve order
form_entry_names = \
       ['firstname','lastname',
        'email','address1','address2','city','state','zip','country',
        'organization','organization2',
        'cellular','tel1','ext1','tel2','ext2','fax1','fax2',
        'web',
        'salutation','honorific','title',
        'performer','status','contact','origin','offstage',
        'comments',
        'jubil04','bene02','purch','promo','appeal','gift','nomo05',
        'wrhndlr','evhndlr','aphndlr','affilother']


input_types = \
    {'comments': 'textarea', 
     'other': 'text'}

dbfile = 'data/rccny_contacts.db'
table = 'contacts'

def PostNewForm():
    PostForm(True)
    
def PostUpdateForm(datadict=None):
    PostForm(False, datadict=datadict)

def PostForm(isnew, datadict=None):

    if datadict is None:
        datadict = {}

    # New recipe or an update?
    if isnew:
        action = "contact_new.py"
    else:
        action = "contact_update.py"

    # Write the form
    print "<form action='"+action+"'>"
    for name in form_entry_names:
        if datadict.has_key(name):
            value = datadict[name]
        else:
            # all new entries are contact='yes' by default
            if name == 'contact':
                value='yes'
            else:
                value = ''
        PostFormField(name,value)
    # if we are updating we need to send the id
    if not isnew:
        print "<input type='hidden' name='id' value='"+str(datadict['id'])+"'>"
    print "<input type='submit' value='Save'>"
    print "</form>"

def PostFormField(name,value):
    if name == 'comments':
        intype = 'textarea'
    else:
        intype = 'text'

    if value != "":
        valstr = " value = '"+value+"'"
    else:
        valstr = ""


    if name == 'ext1' or name=='ext2':
        addstr=''
    else:
        addstr='<br>'
    outstring = name+':'+addstr

    if name != 'tel1' and name != 'tel2':
        addstr='<br>' 
    else:
        addstr=''

    if intype == "text":
        outstring = outstring + \
                "<input size=50 type='text' name='"+name+"'"+valstr+">"+addstr
    else:
        outstring = outstring + \
            "<textarea rows=5 cols=50 name='"+name+"'>"+str(value)+"</textarea>"+addstr

    print outstring

def main(fs):

    print "<body bgcolor='#ffffcc'></body>"

    if not os.path.exists(dbfile):
        print "dbfile does not exist: ",dbfile
        return

    reader = sqlite_reader.Reader(dbfile)

    # If user has sent id we will check if it exists.  If not we will
    # create it as new
    if fs.has_key("id"):
        id = fs['id'].value
        query = "select * from "+table+" where id = "+str(id)
        id_dict = reader.ReadAsDict(query)
        reader.close()
        if len(id_dict) > 0:
            id_dict = id_dict[0]
            PostUpdateForm(datadict=id_dict)
        else:
            print "requested id does not exist: ",id
            return
    else:
        PostNewForm()

    

if __name__=="__main__":
    """
    Create a new recipe from a form.
    """

    print "Content-type: text/html\n\n"

    fs = cgi.FieldStorage()

    main(fs)
