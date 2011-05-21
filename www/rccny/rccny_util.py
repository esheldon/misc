import sys
import os
import sqlite_reader
import urllib

dbfile = 'data/rccny_contacts.db'


def PrintCloseWindow():
    print '<form method="post">'
    print '<input type="button" value="close window" onClick="window.close();">'
    print '</form>'


badsal = ['Friend','Friends']
def GetFullNameLSP(lsp, coldict):

    lname = lsp[coldict['lastname']]
    fname = lsp[coldict['firstname']]
    salutation = lsp[coldict['salutation']]
    email = lsp[coldict['email']].strip()

    if lname == '' and fname == '':
        if salutation == '' or salutation in badsal:
            if email != '':
                name=email
            else:
                name='None'
        else:
            name = salutation
    else:
        name = []
        if lname != '':
            name.append(lname)
        if fname != '':
            name.append(fname)
        name = ', '.join(name)

    name = name.strip()
    if name == '':
        name = 'None'

    return name

def ConvertDate(date):
    import time
    import datetime
    if date.strip() != '':
        try:
            dt = datetime.datetime( *time.strptime(date,'%m/%d/%y')[0:5] )
            dt = dt.strftime('%Y-%m-%d')
            return dt
        except:
            return ''
    else:
        return date



def PrintHead(title):
    print "<Head>"

    print "<link rel='STYLESHEET' href='styles/default.css' type='text/css'>" 
    print "<link rel='STYLESHEET' href='styles/rccny.css' type='text/css'>"
    print "<TITLE>",title,"</TITLE>"
    print "</Head>"

def GetContactQuery(fs, flist=None):
    """
    Return the appropriate query string for generating the main list
    """

    table = 'contacts'
    if fs.has_key('orderby'):
        orderby=fs['orderby'].value
    else:
        orderby='fullname'

    if fs.has_key('sortorder'):
        orderby = orderby + ' ' + fs['sortorder'].value

    if flist is None:
        flist = '*'
    defquery = "select "+flist+" from "+table+" order by "+orderby
    field=None
    message=None
    if fs.has_key("query"):
        # Can enter a full query.  Must return at least the 
        #     fields name,category,id
        # or an error will occur
        query = fs['query'].value

        if fs.has_key('page'):
            nperpage = 50
            page=int(fs['page'].value)
            limit = ' limit %d offset %d' % (nperpage, (page-1)*nperpage)
            query=query + limit

        message= "Search results for query:'"+query+"'"
    elif fs.has_key("field"):
        # Can enter a field and pattern
        if not fs.has_key("pattern"):
            query = defquery
        else:
            if fs['field'].value == 'query':
                field = 'query'
                query = fs['pattern'].value
                message= "Search results for query:'"+query+"'"
            elif fs['field'].value == 'page':
                field = fs['field'].value
                page = int(fs['pattern'].value)
                nperpage = 100
                limit = ' limit %d offset %d' % (nperpage, (page-1)*nperpage)
                query = defquery + limit
                message = 'Page #'+str(page)
            else:
                # build the query
                field = fs['field'].value
                pattern = fs['pattern'].value
                if pattern != "":
                    regexp = ".*"+pattern+".*"
                    message= "Search results for '"+field+"': '"+pattern+"'"
                    query = "select "+flist+" from "+table+" where "+field+" regexp '"+regexp+"' order by "+orderby
                else:
                    query = defquery
    else:
        query = defquery

    return query, field, message

def GetContactData(fs, flist=None):
    dbfile = 'data/rccny_contacts.db'
    if not os.path.exists(dbfile):
        print "dbfile '"+dbfile+"' does not exist"
        sys.exit(45)

    # Open database
    reader = sqlite_reader.Reader(dbfile)

    # Get table query
    query,field,message = GetContactQuery(fs, flist=flist)

    # Get data for each person
    datadict = reader.ReadAsDict(query)
    reader.close()

    return datadict, field, message

def PrintSearchForm(fs, field=None):
    if field is None:
        default_field="alltext"
    else:
        default_field=field

    allfields = {'alltext':'Full Text', 
                 'page':'Page Number',
                 'gift':'Gift Code',
                 'origin':'Origin Code',
                 'contact':'Contact Code',
                 'status':'PerfStatus',
                 'performer':'Performer',
                 'email':'Email',
                 'fullname':'Name',
                 'query':'Query'}
    
    if default_field not in allfields.keys():
        default_field = 'alltext'

    if fs.has_key('pattern'):
        val = fs['pattern'].value.replace("'",'"')
        valstr = " value = '"+val+"'"
    else:
        valstr=''

    print "<form action='rccny_contacts.py'>"
    print "    Search"
    print "    <select name='field'>"
    for f,p in allfields.items():
        if default_field == f:
            print "        <option selected value='"+f+"'>",p
        else:
            print "        <option value='"+f+"'>",p
    print "    </select>"
    print "    <input size=30 type='text' name='pattern'"+valstr+">"
    print "    <input type='submit' value='Submit'>"
    print "</form>"



def PrintSearchFormOld(fs, field=None):
    if field is None:
        default_field="alltext"
    else:
        default_field=field

    allfields = {'alltext':'Full text', 
                 'gift':'Gift Code',
                 'contact':'Contact Code',
                 'status':'PerfStatus',
                 'performer':'Performer',
                 'email':'Email',
                 'fullname':'Name',
                 'query':'Query'}
    
    if default_field not in allfields.keys():
        default_field = 'alltext'

    if fs.has_key('pattern'):
        val = fs['pattern'].value.replace("'",'"')
        valstr = " value = '"+val+"'"
    else:
        valstr=''

    print "<form action='rccny_contacts.py'>"
    print "    Search"
    print "    <select name='field'>"
    for f,p in allfields.items():
        if default_field == f:
            print "        <option selected value='"+f+"'>",p
        else:
            print "        <option value='"+f+"'>",p
    print "    </select>"
    print "    <input size=30 type='text' name='pattern'"+valstr+">"
    print "    <input type='submit' value='Submit'>"
    print "</form>"

badsal = ['Friend','Friends']
def GetName(row, lastfirst=True):
    lname = row['lastname']
    fname = row['firstname']
    name = []

    if fname =='' and lname == '':
        sal = row['salutation']
        if sal in badsal:
            if row['email'] == '':
                name = 'None'
            else:
                name = row['email']
        else:
            name = row['salutation']
    else:
        
        if lastfirst:
            if lname != '':
                name.append(lname)
            if fname != '':
                name.append(fname)
            name = ', '.join(name)
        else:
            if fname != '':
                name.append(fname)
            if lname != '':
                name.append(lname)
            name = ' '.join(name)

    return name
 

# The <th> list and some properties.  The order here is the order they will appear
# in the main table
thlist = [('fullname','Name'),
          ('modified','Modified'),
          ('entered','Entered'),
          ('gift','Gift'),
          ('origin','Origin'),
          ('organization','Org'),
          ('contact','Contact'),
          ('performer','Performer'),
          ('status','PStatus'),
          ('email','Email')]
default_sortorder = \
    {'fullname':'ASC',
     'modified': 'DESC',
     'entered':'DESC',
     'gift': 'DESC',
     'origin':'DESC',
     'organization':'DESC',
     'contact': 'DESC',
     'performer': 'DESC',
     'status': 'DESC',
     'email': 'DESC',
     'default': 'ASC'}
default_orderby = 'fullname'


def GetColumnSortDict(fs):
    """
    The sort order in the th column header url
    """

    if fs.has_key('orderby'):
        current_orderby = fs['orderby'].value
    else:
        current_orderby = default_orderby
    if fs.has_key('sortorder'):
        current_sortorder = fs['sortorder'].value
    else:
        if default_sortorder.has_key(current_orderby):
            current_sortorder = default_sortorder[current_orderby]
        else:
            current_sortorder = default_sortorder['default']

    sodict = default_sortorder.copy()
    if sodict.has_key(current_orderby):
        if current_sortorder.upper() == 'ASC':
            sodict[current_orderby] = 'DESC'
        else:
            sodict[current_orderby] = 'ASC'
    else:
        sodict[current_orderby] = sodict['default']

    return sodict

   

def PrintTableByName(datadict, fs, caption=None):
    """
    Prints the results by name and catagory
    """

    print " <table width='100%' class='main'>"
    if caption is not None:
        print "    <caption>",caption,"</caption>"

    print "<br> Found ",len(datadict)," entries"
    # make sure we pass on the current selection to different sortings
    extra = []
    if len(fs) > 0:
        for k in fs.keys():
            if k != 'orderby' and k != 'sortorder':
                extra.append((k,fs[k].value))
                #extra.append(k+'='+fs[k].value)

    extra = urllib.urlencode(extra)
    #extra = '&'.join(extra)
    if extra != '':
        extra = '&'+extra

    sodict = GetColumnSortDict(fs)
    urldict = {}
    for th,tit in thlist:
        textra = extra+'&sortorder='+urllib.quote_plus(sodict[th])
        urldict[th] = "<a href='rccny_contacts.py?orderby="+th+textra+"'>"+tit+"</a>" 


    print "   <tr>"
    print "     <th>",urldict['fullname'],"</th>"
    print "     <th width='10%' style='text-align:center'>",urldict['modified'],"</th>"
    print "     <th width='10%' style='text-align:center'>",urldict['entered'],"</th>"
    print "     <th width='10%' style='text-align:center'>",urldict['gift'],"</th>"
#    print "     <th width='10%' style='text-align:center'>",urldict['origin'],"</th>"
#    print "     <th width='10%' style='text-align:center'>",urldict['organization'],"</th>"
    print "     <th width='10%' style='text-align:center'>",urldict['contact'],"</th>"
    print "     <th width='15%' style='text-align:center'>",urldict['performer'],"</th>"
    print "     <th width='10%' style='text-align:center'>",urldict['status'],"</th>"
    print "     <th>",urldict['email'],"</th>"
    print "  </tr>"
    
    """
    print "   <tr>"
    print "     <th>",urldict['fullname'],"</th>"
    print "     <th style='text-align:center'>",urldict['entered'],"</th>"
    print "     <th style='text-align:center'>",urldict['gift'],"</th>"
    print "     <th style='text-align:center'>",urldict['contact'],"</th>"
    print "     <th style='text-align:center'>",urldict['performer'],"</th>"
    print "     <th style='text-align:center'>",urldict['status'],"</th>"
    print "     <th>",urldict['email'],"</th>"
    print "  </tr>"
    """

    for row in datadict:
        id = row['id']
        url = "contact2html.py?id="+str(id)

        lname = row['lastname']
        fname = row['firstname']
        modified = row['modified']
        entered = row['entered']

        name = GetName(row)
        namecol = "<a href='"+url+"' target='_blank'>"+name+"</a>"

        # The performance status
        perf = row['performer']
        if perf == '':
            perfurl = ''
        else:
            perfurl = "<a href='rccny_contacts.py?field=performer&pattern="+perf+"'>"+perf+"</a>"
        status = row['status']
        pstatus = status.replace(" ","&nbsp;")
        if status == '':
            status = 'None'
            staturl = ''
        else:
            staturl = "<a href='rccny_contacts.py?field=status&pattern="+status+"'>"+pstatus+"</a>"

        gift = row['gift']
        gurl = "<a href='rccny_contacts.py?field=gift&pattern="+gift+"'>"+gift+"</a>"

#        orig = row['origin']
#        origurl = "<a href='rccny_contacts.py?field=origin&pattern="+orig+"'>"+orig+"</a>"

#        org = row['organization']
#        orgurl = "<a href='rccny_contacts.py?field=organizaion&pattern="+org+"'>"+org+"</a>"


        ccall = row['contact']

        curl = "<a href='rccny_contacts.py?field=contact&pattern="+ccall+"'>"+ccall+"</a>"
        email = row['email']
        eurl = "<a href='mailto:"+email+"'>"+email+"</a>"
        print "  <tr>"
        print "    <td>",namecol,"</td>"
        print "    <td style='white-space:nowrap; text-align:center'>",modified,"</td>"
        print "    <td style='white-space:nowrap; text-align:center'>",entered,"</td>"
        print "    <td style='text-align:center'>",gurl,"</td>"
#        print "    <td style='text-align:center'>",origurl,"</td>"
#        print "    <td style='text-align:center'>",orgurl,"</td>"
        print "    <td style='text-align:center'>",curl,"</td>"
        print "    <td style='text-align:center'>",perfurl,"</td>"
        print "    <td style='text-align:center'>",staturl,"</td>"
        print "    <td>",eurl,"</td>"
        print "  </tr>"

    print "</table>"



   
def GetAddress(datadict, addname=False):

    address = []

    if addname:
        name = GetName(datadict, lastfirst=False)
        address.append(name)

    # address elements
    a1 = datadict['address1']
    a2 = datadict['address2']
    if a1 != '':
        address.append(a1)
    if a2 != '':
        address.append(a2)

    city = datadict['city']
    state = datadict['state']
    zip = datadict['zip']
    country = datadict['country']
    
    line = ''
    if city != '':
        line = city+', '
    line = line + state+' '+zip+' '+country 
    line = line.strip()

    if line != '':
        address.append(line)

    return address

def GetAddressElements(datadict):
    address = []

    name = GetName(datadict, lastfirst=False)
    address.append(name)

    address.append(datadict['address1'])
    address.append(datadict['address2'])

    #city = datadict['city']
    #state = datadict['state']
    #zip = datadict['zip']
    #country = datadict['country']
    
    address.append(datadict['city'])
    address.append(datadict['state'])
    address.append(datadict['zip'])
    #address.append(datadict['country'])

    return address

checkfields = [1,2,3,4,5]
def CheckUniqueAddress(address):
    a = [address[i].strip() for i in checkfields]
    a = ' '.join(a)

reqfields = [0,1,3,4,5]
def PrintAddressCSV(datadict, nel=6):
    address = GetAddressElements(datadict)
    if len(address) != nel:
        return
    for rf in reqfields:
        if address[rf].strip() == '':
            return
    address = ','.join(address)
    if address != '':
        print address

reqfields = [0,1,3,4,5]
def PrintAddressTab(datadict, nel=6):
    address = GetAddressElements(datadict)
    if len(address) != nel:
        return
    for rf in reqfields:
        if address[rf].strip() == '':
            return
    address = '\t'.join(address)
    if address != '':
        print address

def PrintAddressHuman(datadict, nel=6):
    address = GetAddressElements(datadict)
    if len(address) != nel:
        return None
    for rf in reqfields:
        if address[rf].strip() == '':
            return None
    address = ''.join(address)
    if address.strip() == '':
        return None
    print GetName(datadict, lastfirst=False)
    add1 = datadict['address1'].strip()
    if add1 != '':
        print add1
    add2 = datadict['address2'].strip()
    if add2 != '':
        print add2
    print datadict['city']+', '+datadict['state']+'  '+datadict['zip']
    return 1



def PrintAddressTD(datadict):

    address = GetAddress(datadict)
    address = '<br>'.join(address)
    if address != '':
        print "        <tr><td><em>Address:</em></td><td>",address,"</td></tr>"



