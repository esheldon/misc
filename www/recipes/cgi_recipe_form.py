#!/usr/bin/env python2.5

import os
import re

import cgi

import recipe_util


reg = {}
reg['item'] = re.compile('.*item.*')
reg['step'] = re.compile('.*step.*')

# this is a list to preserve order
form_entry_names = \
        ['name','category','servings','notes',
         'item1','item2','item3','item4','item5','item6','item7','item8',
         'item9','item10','item11','item12','item13','item14','item15',
         'item16','item17','item18',
         'step1','step2','step3','step4','step5','step6','step7','step8',
         'step9','step10','step11','step12','step13','step14','step15',
         'step16','step17','step18',
         'comments']

input_types = \
    {'name': 'text', 
     'category': 'text', 
     'servings': 'text', 
     'notes': 'textarea',
     'item': 'text',
     'step': 'text',
     'comments': 'textarea'}

def PostNewForm(dbfile):
    PostForm(dbfile,True)
    
def PostUpdateForm(dbfile, datadict=None):
    PostForm(dbfile, False, datadict=datadict)

def PostForm(dbfile, isnew, datadict=None):

    if datadict is None:
        datadict = {}

    # New recipe or an update?
    if isnew:
        action = "cgi_new_recipe.py"
    else:
        action = "cgi_update_recipe.py"

    # Write the form
    print "<form action='"+action+"'>"
    for name in form_entry_names:
        if datadict.has_key(name):
            value = datadict[name]
        else:
            value = ''
        PostFormField(name,value)
    # if we are updating we need to send the id
    if not isnew:
        print "<input type='hidden' name='id' value='"+str(datadict['id'])+"'>"
    print "<input type='hidden' name='dbfile' value='"+dbfile+"'>"
    print "<input type='submit' value='Submit'>"
    print "</form>"

def PostFormField(name,value):
    if reg['item'].match(name):
         intype = input_types['item']
    elif reg['step'].match(name):
         intype = input_types['step']
    else:
         intype = input_types[name]

    if value != "":
        valstr = " value = '"+value+"'"
    else:
        valstr = ""

    outstring = name+':<br>'
    if intype == "text":
        outstring = outstring + \
                "<input size=50 type='text' name='"+name+"'"+valstr+"><br>"
    else:
        outstring = outstring + \
            "<textarea rows=5 cols=50 name='"+name+"'>"+str(value)+"</textarea><br>"

    print outstring

def main(fs):

    print "<body bgcolor='#ffffcc'></body>"


    if not fs.has_key("dbfile"):
        print "You must send a dbfile to this script"
        return

    dbfile = fs['dbfile'].value

    # If user has sent id we will check if it exists.  If not we will
    # create it as new
    if fs.has_key("id"):
        id = fs['id'].value
    else:
        id=None

    recipe=recipe_util.Form(dbfile, id)
    recipe.post()

    crap="""
        query = "select * from recipes where id = "+str(id)
        id_dict = reader.ReadAsDict(query)
        if len(id_dict) > 0:
            id_dict = id_dict[0]
            PostUpdateForm(dbfile,datadict=id_dict)
        else:
            print "requested id does not exist: ",id
            return
    else:
        PostNewForm(dbfile)

    reader.close()
    """
    

if __name__=="__main__":
    """
    Create a new recipe from a form.
    """

    print "Content-type: text/html\n\n"

    fs = cgi.FieldStorage()

    main(fs)
