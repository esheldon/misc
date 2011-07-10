import re
import os
import sys
import sqlite3 as sqlite
import urllib

# this does not include
db_names_inorder = \
        ['id','name','category',
         'notes','comments','servings',
         'item1','item2','item3','item4','item5','item6','item7','item8',
         'item9','item10','item11','item12','item13','item14','item15',
         'item16','item17','item18',
         'step1','step2','step3','step4','step5','step6','step7','step8',
         'step9','step10','step11','step12','step13','step14','step15',
         'step16','step17','step18','alltext']

db_recipe_tabledef = \
        """CREATE TABLE recipes
                (id integer primary key,
                 name text,
                 category text,
                 notes text,
                 comments text,
                 servings text,
                 item1 text,
                 item2 text,
                 item3 text,
                 item4 text,
                 item5 text,
                 item6 text,
                 item7 text,
                 item8 text,
                 item9 text,
                 item10 text,
                 item11 text,
                 item12 text,
                 item13 text,
                 item14 text,
                 item15 text,
                 item16 text,
                 item17 text,
                 item18 text,
                 step1 text,
                 step2 text,
                 step3 text,
                 step4 text,
                 step5 text,
                 step6 text,
                 step7 text,
                 step8 text,
                 step9 text,
                 step10 text,
                 step11 text,
                 step12 text,
                 step13 text,
                 step14 text,
                 step15 text,
                 step16 text,
                 step17 text,
                 step18 text,
                 alltext text)
                 """

def db_dir():
    #return '/srv/wwwhosts/www.cosmo.bnl.gov/SQLite/esheldon/recipes'
    #return 'old_db'
    return 'data'

def db_path(name):
    dbdir=db_dir()
    return os.path.join(dbdir,os.path.basename(name))


def regexp(expr, item):
    """
    Return 1 if `item` matches the regular expression `expr`, 0 otherwise
    """
    r = re.compile(expr,re.IGNORECASE)
    return r.match(item) is not None


def getconn(dbfile):
    # we force the directory to be db_dir()
    dbpath = db_path(dbfile)

    if not os.path.exists(dbpath):
        print "dbfile '"+dbfile+"' does not exist"
        sys.exit(45)

    try:
        conn = sqlite.connect(dbpath)
        conn.row_factory = sqlite.Row
        conn.create_function("regexp", 2, regexp)
    except:
        sys.stdout('Could not connect to dbfile: "%s"<br>' % dbfile)
        sys.exit(45)

    return conn

_dbpattern = re.compile('.*\.db$')
def db_file_list():
    dbdir = db_dir()
    allfiles = os.listdir(dbdir)
    dbfiles = [] 
    for f in allfiles:
        if _dbpattern.match(f):
            dbfiles.append(f)
    return dbfiles

def name2file(name):
    """
    Make a unix ready name.  Must be a local directory only.
    """
    newname = os.path.basename(name)
    newname = newname.replace("'","")
    newname = newname.replace(" ","_")
    return newname+'.db'




def create_db(name, owner):
    # Create a simple unix name
    dbfile = name2file(name)

    dbpath=db_path(dbfile)

    # Make sure it doesn't already exist
    if os.path.exists(dbpath):
        print "Book %s (%s) already exists" % (name,dbfile)
        sys.exit(45)

    try:

        conn = sqlite.connect(dbpath, isolation_level=None)
        curs = conn.cursor()

        curs.execute('create table meta (name text, owner text)')
        curs.execute('insert into meta values (?, ?)', (name, owner))
        curs.execute(db_recipe_tabledef)
        print "Successfully added recipe book: ",name,"<br>" 
        print "<a href='recipes.py'>Back to main page</a><br>"
        conn.close()
    except:
        print "Failed to create recipe book: ",name,"<br>"
        print "Error info: ",sys.exc_info(),"<br>"


def delete_db(dbfile, confirm=True):
    dbpath=db_path(dbfile)
    if not os.path.exists(dbpath):
        print "Recipe book: %s does not exist" % dbfile
        sys.exit(45)

    if confirm:
        mess="""
        Are you sure you want to delete Recipe Book %s?
        <form action=cgi_deletedb.py>
          <input type='hidden' name='dbfile' value='%s'>
          <input type='submit' name='confirm' value='yes'>
          <input type='submit' name='confirm' value='no'>
        </form>
        """ % (dbfile,dbfile)
        print mess
    else:
        import shutil
        import time
        t = str(int(time.time()))
        new_dbfile = dbfile+'.'+t
        new_dbfile = os.path.join('trash', new_dbfile)
        try:
            shutil.move(dbpath,new_dbfile)
            print "Recipe Book '%s' was deleted<br>" % dbfile
            print "<a href='recipes.py'>Back to Book List</a><br>"
        except:
            print "Error deleting recipe book'%s'" % dbfile
            print "Error info: ",sys.exc_info(),"<br>"




def print_recipe_book_stats():

    print_header("Recipe Database")
    print "<table width='100%' cellpadding=0 border=0>"
    print "  <tr>"
    print "    <td><font size=6>Recipe Books</font></td>"
    print "    <td align='right'><a href='cgi_newdb_form.py'>Add a new recipe book</a></td>"
    print "  </tr>"
    print "</table>"

    print "<p>"
    print "<table width='100%' class='main'>"
    print "<tr><th class='main'>Name</th><th class='main'># Recipes</th><th class='main'>Owner</th></tr>"

    dbfiles=db_file_list()
    i=1
    for f in dbfiles:
        fpath = db_path(f)
        reader = SqliteReader(f)
        res = reader.execute('select name,owner from meta limit 1')
        if len(res) > 0:
            name=res[0]['name']
            owner=res[0]['owner']
        else:
            name='Unknown'
            owner='Unknown'

        res = reader.execute('select count(*) from recipes')
        if len(res) > 0:
            nrecipe = res[0][0]
        else:
            nrecipe = 0

        url = "recipe_list.py?dbfile="+f
        print "<tr><td class='main'><a href='%s'>%s</a></td><td class='main'>%d</td><td class='main'>%s</td></tr>" % (url,name,nrecipe,owner)


    print "</table>"


def print_recipe_list(fs):
    # Required field: dbfile
    if not fs.has_key('dbfile'):
        print "You must enter a dbfile"
        return

    dbfile = str(fs['dbfile'].value)
    # Open database and get some metadata
    reader = SqliteReader(dbfile)

    nquery = "select name from meta"
    try:
        res = reader.execute(nquery)
    except:
        print "error occurred: '%s'" % nquery
        return
    if len(res) == 0:
        name = 'Unknown'
    else:
        name = res[0]['name']

    # Get table query
    query,field,message = GetMainTableQuery(fs)
    # Get data for each recipe
    datadict = reader.execute(query)

    print_header(name)

    print "<table width='100%' cellpadding=0 border=0>"
    print "  <tr>"
    print "    <td><font size=6><a href='recipe_list.py?dbfile="+dbfile+"'>",name,"</a></font></td>"
    mcall = "<a href='recipes.py'>Back to book list</a>"
    ncall = "<a href='cgi_recipe_form.py?dbfile="+dbfile+"'>Add a new recipe</a>"
    dcall = "<a href='cgi_deletedb.py?dbfile="+dbfile+"'>Delete Recipe Book</a>"
    calls = mcall + ' | '+ncall + ' | '+dcall
    print "    <td align='right'>",calls,"</td>"
    print "  </tr>"
    print "</table>"

    PrintSearchForm(fs,field=field)
    PrintTableByName(dbfile,datadict,tclass='main', caption=message)


def print_header(title):
    print "<Head>"

    print "<link rel='STYLESHEET' href='styles/default.css' type='text/css'>" 
    print "<link rel='STYLESHEET' href='styles/recipe.css' type='text/css'>"
    print "<TITLE>",title,"</TITLE>"
    print "</Head>"

def GetMainTableQuery(fs):
    """
    Return the appropriate query string for generating the main recipe
    table list
    """
    defquery = "select name, category, id, notes from recipes order by category"
    field=None
    message=None
    if fs.has_key("query"):
        # Can enter a full query.  Must return at least the 
        #     fields name,category,id
        # or an error will occur
        query = fs['query'].value
        message= "Search results for query:'"+query+"'"
    elif fs.has_key("field"):
        # Can enter a field and pattern
        if not fs.has_key("pattern"):
            #message= "Error: need both field and pattern. Returning all"
            query = defquery
        else:
            # build the query
            field = fs['field'].value
            pattern = fs['pattern'].value
            if pattern != "":
                regexp = ".*"+pattern+".*"
                message= "Search results for '"+field+"': '"+pattern+"'"
                query = "select name,category,id,notes from recipes where "+field+" regexp '"+regexp+"'"
            else:
                query = defquery
    else:
        query = defquery

    return query, field, message

def PrintSearchForm(fs, field=None):
    if field is None:
        default_field="alltext"
    else:
        default_field=field

    allfields = {'alltext':'Full text', 'category':'Category','name':'Name'}
    if default_field not in allfields.keys():
        default_field = 'alltext'

    if fs.has_key('pattern'):
        val = fs['pattern'].value.replace("'",'"')
        valstr = " value = '"+val+"'"
    else:
        valstr=''

    dbfile=fs['dbfile'].value
    print "<form action='recipe_list.py'>"
    print "    Search"
    print "    <select name='field'>"
    for f,p in allfields.items():
        if default_field == f:
            print "        <option selected value='"+f+"'>",p
        else:
            print "        <option value='"+f+"'>",p
    print "    </select>"
    print "    <input size=30 type='text' name='pattern'"+valstr+">"
    print "    <input type='hidden' name='dbfile' value='"+dbfile+"'>"
    print "    <input type='submit' value='Submit'>"
    print "</form>"



def PrintSearchFormOld(dbfile, field=None):
    if field is None:
        default_field="alltext"
    else:
        default_field=field

    allfields = {'alltext':'Full text', 'category':'Category','name':'Name'}
    if default_field not in allfields.keys():
        default_field = 'alltext'

    print "<form action='recipe_list.py'>"
    print "    Search"
    print "    <select name='field'>"
    for f,p in allfields.items():
        if default_field == f:
            print "        <option selected value='"+f+"'>",p
        else:
            print "        <option value='"+f+"'>",p
    print "    </select>"
    print "    <input type='text' name='pattern'>"
    print "    <input type='hidden' name='dbfile' value='"+dbfile+"'>"
    print "    <input type='submit' value='Submit'>"
    print "</form>"


def PrintTableByName(dbfile, datadict, caption=None,tclass=None):
    """
    Prints the results by name and catagory
    """
    if tclass is None:
        tclass=""

    print " <table width='100%' class='"+tclass+"'>"
    if caption is not None:
        print "    <caption>",caption,"</caption>"
    print "   <tr><th class='"+tclass+"'>Name</th><th class='"+tclass+"'>Category</th><th class='"+tclass+"'>Notes</th></tr>"

    for row in datadict:
        id = row['id']
        url = "cgi_recipe2html.py?id="+str(id)+"&dbfile="+dbfile
        namecol = "<a href='"+url+"'>"+row['name']+"</a>"
        cat = row['category']
        pcat = cat.replace(" ","&nbsp;")

        caturl = "<a href='recipe_list.py?dbfile="+dbfile+"&field=category&pattern="+cat+"'>"+pcat+"</a>"
        notecol = row['notes']
        print "  <tr><td class='"+tclass+"'>",namecol,"</td><td class='"+tclass+"'>",caturl,"</td><td class='"+tclass+"'>",notecol,"</td></tr>"

    print "</table>"

class RowWrapper(object):
    """

    Make up for the deficiencies of the python2.5 Row class by
    defining a keys() function

    """
    def __init__(self, row, keys):
        self._row = row
        self._keys = keys
    def __getitem__(self, key):
        """
        We still get the name/number lookup
        """
        return self._row[key]

    def has_key(self, key):
        return key in self._keys

    def keys(self):
        return self._keys


def cursor_keys(cursor):
    keys=[]
    for d in cursor.description:
        keys.append(d[0])
    return keys

class SqliteReader:
    """
    Read from an sqlite database 
    """

    def __init__(self, dbfile):
        self.dbfile = dbfile
        self.conn = getconn(dbfile)
        self.curs = self.conn.cursor()

    def execute(self, query, keys=False):
        self.curs.execute(query)
        res = self.curs.fetchall()

        keylist=[]
        if len(res) < 1:
            res=[]
        if keys:
            keylist=cursor_keys(self.curs)

        if keys:
            return res, keylist
        else:
            return res

_typereg = {}
_typereg['item'] = re.compile('.*item.*')
_typereg['step'] = re.compile('.*step.*')


class Recipe(object):
    def __init__(self, dbfile=None, id=None, load=False):
        if dbfile is not None:
            self.dbfile = dbfile
        else:
            self.dbfile=None

        if id is not None:
            self.id = id
        else:
            self.id = None

        if load:
            self.load(dbfile, id)


    def load(self, dbfile=None, id=None):
        # can override current settings
        if dbfile is not None:
            self.dbfile = dbfile
        if id is not None:
            self.id = id

        if self.dbfile is None or self.id is None:
            raise ValueError('dbfile and id must be set')
            
        reader = SqliteReader(self.dbfile)

        query="select * from recipes where id=%s" % self.id
        res, keys = reader.execute(query, keys=True)
        if len(res) != 1:
            raise ValueError,\
                'No recipe found in %s for id=%s' % (self.dbfile,self.id)

        self.recipe = RowWrapper(res[0], keys)

        query="select name from meta"
        res = reader.execute(query)
        if len(res) == 0:
            raise ValueError,\
                'Could not read name from meta table in %s'  % (self.dbfile,)
        self.dbtitle = res[0]['name']


    def delete(self, dbfile=None, id=None, confirm=True):
        # can override current settings
        if dbfile is not None:
            self.dbfile = dbfile
        if id is not None:
            self.id = id

        if self.dbfile is None:
            raise ValueError('dbfile must be set')
        if self.id is None and not create:
            raise ValueError('id must be set unless create=True')

        conn = sqlite.connect(db_path(self.dbfile), isolation_level=None)
        curs = conn.cursor()

        # load this recipe
        self.load()
        name = self.recipe['name']

        # If confirm=yes is sent then we will delete, else not
        if confirm:

            mess="""
            Are you sure you want to delete the recipe %s?
            <form action=cgi_delete_recipe.py>
              <input type='hidden' name='dbfile' value='%s'>
              <input type='hidden' name='id' value='%s'>
              <input type='submit' name='confirm' value='yes'>
              <input type='submit' name='confirm' value='no'>
            </form>
            """ % (name, self.dbfile, self.id)

            print mess
        else:
            # build up delete query
            query = "delete from recipes where id = ?"
            try:
                curs.execute(query, (self.id,))
                print "Recipe '%s' was deleted<br>" % name
                curs.execute('select name from meta')
                res = curs.fetchall()
                if len(res) > 0:
                    dbname = res[0][0]
                else:
                    dbname = 'main page'
                print "<a href='recipe_list.py?dbfile="+self.dbfile+"'>Back to ",dbname,"</a><br>"
            except:
                print "Error deleting recipe '%s'" % name
                print "Error info: ",sys.exc_info(),"<br>"


    def create(self, 
               form=None, 
               data=None,
               dbfile=None):
        self.update(form=form,data=data,dbfile=dbfile,create=True)

    def update(self, 
               form=None, 
               data=None,
               dbfile=None, 
               id=None,
               create=False):
        """

        Update a database entry. Send data either as a cgi form= or a
        dictionary with data=.

        The form keyword will take precedence.

        """

        # can override current settings
        if dbfile is not None:
            self.dbfile = dbfile
        if id is not None:
            self.id = id

        if self.dbfile is None:
            raise ValueError('dbfile must be set')
        if self.id is None and not create:
            raise ValueError('id must be set unless create=True')

        if form is not None:
            data=form
        if data is None:
            raise ValueError("Enter form= cgi form or data= dictionary")

        # used when updating
        insert_vals = []
        string_vals = []
        setpairs = []

        # used when creating
        all_vals = []

        # loop over all and only check the matches
        for name in db_names_inorder:
            if name == "id":
                val=None
            elif name == "alltext":
                val=None
            else:
                if data_haskey(data, name):
                    # This assumes all fields are text except id
                    val = str( data_keyvalue(data, name) )
                    setpairs.append(name+'=?')
                    insert_vals.append(val)
                    string_vals.append(val)
                else:
                    # We need to put None's in so we can *delete* fields.
                    val = ''
                    setpairs.append(name+'=?')
                    insert_vals.append(val)

            all_vals.append(val)

        alltext = " ".join(string_vals)
        insert_vals.append(alltext)
        setpairs.append('alltext=?')

        if create:
            all_vals[-1] = alltext
            qmarks = ['?']*len(all_vals)
            qmarks = ",".join(qmarks)

            ps = 'insert into recipes values('+qmarks+')'
            action='create'
        else:

            # set up the ?=? pairs
            setpairs = ",".join(setpairs)

            # prepared statement
            ps = "update recipes set "+setpairs+" where id = ?"
            insert_vals.append(self.id)

            action='update'

        if not data_haskey(data, 'name'):
            recipe_name = 'Noname'
        else:
            recipe_name = data_keyvalue(data, 'name')

        # set isolation level so that the database update will work
        conn = sqlite.connect(db_path(self.dbfile), isolation_level=None)
        curs = conn.cursor()

        try:
            if not create:
                curs.execute(ps, insert_vals)
                print "Successfully updated recipe: ",recipe_name,"<br>" 

                url = "cgi_recipe2html.py?id="+str(self.id)+"&dbfile="+self.dbfile
                print "<a href='"+url+"'>View ",recipe_name,"</a><br>"

            else:
                # creating recipe
                curs.execute(ps, all_vals)
                print "Successfully added recipe: ",recipe_name,"<br>" 

            curs.execute('select name from meta')
            res = curs.fetchall()
            if len(res) > 0:
                dbname = res[0][0]
            else:
                dbname = 'main page'
            print "<a href='recipe_list.py?dbfile="+self.dbfile+"'>Back to ",dbname,"</a><br>"


        except:
            print "Failed to",action,"recipe: ",recipe_name,"<br>"
            print "Error info: ",sys.exc_info(),"<br>"

        conn.close()





    def tohtml(self, doprint=False):
        print_header(self.dbtitle) 
        self.write_main_table(doprint=doprint)


    def write_main_table(self, doprint=False):
        """
        doprint=True for a printable version, no color background
        """
        if doprint:
            tclass='recipe_print'
        else:
            tclass='recipe'

        border = "0"

        name=self.recipe['name']
        category=self.recipe['category']
        servings=self.recipe['servings']
        notes=self.recipe['notes']

        ucall = "cgi_recipe_form.py?id="+str(self.id)+"&dbfile="+self.dbfile
        ucall = "<a href='"+ucall+"'>Edit</a>"
        uname = name.replace("'", "")
        uname = urllib.quote_plus(uname)
        dcall = "cgi_delete_recipe.py?id="+str(self.id)+"&name="+uname+"&dbfile="+self.dbfile
        dcall = "<a href='"+dcall+"'>Delete</a>"
        pcall = "<a href='cgi_recipe2html.py?id="+str(self.id)+"&dbfile="+self.dbfile+"&print=yes' target='_blank'>Printer Friendly Version</a>"

        print "<table>"
        #print "   <tr><td align='center'><font size=5>Recipe</font></td></tr>"
        mainurl = "<a href='recipe_list.py?dbfile="+self.dbfile+"'>"+self.dbtitle+"</a>"
        editurl = ucall+" | "+dcall + " | "+pcall
        # A Header with recipe book name, edit, delete, printable
        if not doprint:
            print  "   <tr><td widht='100%'>"
            print "      <table width='100%' border="+border+">"
            print "       <tr><td align='left'><font size=5>"+mainurl+"</font></td><td align='right'>"+editurl+"</td></tr>"
            print "      </table>"
            print "   </td></tr>"
        print "   <tr><td>"
        print "      <table class='"+tclass+"' border="+border+">"
        print "        <tr><td><em>Name:</em></td><td><b>",name,"</b></td></tr>"
        print "        <tr><td><em>Category:</em></td><td>",category,"</td></tr>"
        print "        <tr><td><em>Quantity:</em></td><td>",servings,"</td></tr>"
        print "        <tr><td><em>Notes:</em></td><td>",notes,"</td></tr>"
        #print "        <tr><td><em>Modify:</em></td><td>",ucall,dcall,"</td></tr>"
        print "        <tr><td>&nbsp;</td><td>&nbsp;</td></tr>"


        self.write_items("item", "Ingredients",twocol=True, include_number=False)
        self.write_items("step", "Directions")
        self.write_comments()
        print "     </table>" 
        print "  </td></tr>"
        print "</table>"

    def write_items(self, stype, typename, twocol=False, include_number=True):

        # row with instructions title and nothing in next column
        print "        <tr><td><em><b>",typename,"</b></em></td><td></td></tr>"
     
        # if twocol then write the empty row first and start
        # a sub-table
        if twocol:
            print "        <tr>"
            print "            <td></td>"
            print "            <td>"
            print "                <table width='100%' cellpadding=0 border=0>"

            
        #cl='underline'
        cl=''
        i=0
        itemnum=1
        icol=1
        for key in self.recipe.keys():
            if _typereg[stype].match(key):
                # this is one of the types we will print
                if self.recipe[key] != "":
                    # not-empty
                    if not include_number:
                        idesc = ""
                    else:
                        idesc = str(itemnum)+". "
                        itemnum=itemnum+1
                    pp = idesc+self.recipe[key]
                    if twocol:
                        if icol == 1:
                            print "                <tr>"
                            print "                    <td width='50%' class='"+cl+"'>"+pp+"</td>"
                            icol=2
                        else:
                            print "                    <td width='50%' class='"+cl+"'>"+pp+"</td>"
                            print "                </tr>"
                            icol=1
                    else:
                        print "        <tr><td></td><td class='"+cl+"'>"+pp+"</td></tr>"
            i=i+1
     
        if twocol:
            print "                </table>"
            print "            </td>"
            print "        </tr>"


    def write_comments(self):
        comm = self.recipe['comments']
        print "        <tr><td><em><b>Comments</b></em></td><td></td></tr>"
        print "        <tr><td></td><td>",comm,"</td></tr>"





class Form(object):
    def __init__(self, dbfile, id=None):
        self.dbfile=dbfile
        self.id=id

        # if id is sent, this is supposed to be an
        # update form
        self.recipe=None
        if id is not None:
            self.load()

        # this is a list to preserve order
        self.form_entry_names = \
            ['name','category','servings','notes',
             'item1','item2','item3','item4','item5','item6','item7','item8',
             'item9','item10','item11','item12','item13','item14','item15',
             'item16','item17','item18',
             'step1','step2','step3','step4','step5','step6','step7','step8',
             'step9','step10','step11','step12','step13','step14','step15',
             'step16','step17','step18',
             'comments']
        self.input_types = \
                {'name': 'text', 
                 'category': 'text', 
                 'servings': 'text', 
                 'notes': 'textarea',
                 'item': 'text',
                 'step': 'text',
                 'comments': 'textarea'}


    def load(self):
        reader = SqliteReader(self.dbfile)

        query="select * from recipes where id=%s" % self.id
        print 'query=',query
        res, keys = reader.execute(query, keys=True)
        if len(res) != 1:
            raise ValueError,\
                'No recipe found in %s for id=%s' % (self.dbfile,self.id)

        self.recipe = RowWrapper(res[0], keys)

    def post(self):
        if self.recipe is None:
            isnew=True
        else:
            isnew=False

        if self.recipe is None:
            datadict = {}
        else:
            datadict=self.recipe

        # New recipe or an update?
        if isnew:
            action = "cgi_new_recipe.py"
        else:
            action = "cgi_update_recipe.py"

        # Write the form
        print "<form action='"+action+"'>"
        for name in self.form_entry_names:
            if datadict.has_key(name):
                value = datadict[name]
            else:
                value = ''
            self.post_field(name,value)
        # if we are updating we need to send the id
        if not isnew:
            print "<input type='hidden' name='id' value='"+str(datadict['id'])+"'>"
        print "<input type='hidden' name='dbfile' value='"+self.dbfile+"'>"
        print "<input type='submit' value='Submit'>"
        print "</form>"

    def post_field(self, name, value):
        if _typereg['item'].match(name):
             intype = self.input_types['item']
        elif _typereg['step'].match(name):
             intype = self.input_types['step']
        else:
             intype = self.input_types[name]

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

def data_haskey(data, key):
    if isinstance(data, dict):
        return (key in data)
    else:
        return data.has_key(key)

def data_keyvalue(data, key):
    if isinstance(data, dict):
        return data[key]
    else:
        return data[key].value
