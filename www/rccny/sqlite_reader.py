import sqlite3 as sqlite
import os
import re
import traceback

class Reader:
    """
    Read from an sqlite database 
    """

    def __init__(self, dbfile):
        self.dbfile = dbfile

        self.conn = sqlite.connect(dbfile)

        # for searching with the regexp function 
        self.conn.create_function("regexp", 2, self.regexp)

        self.conn.text_factory = str

    def close(self):
        self.conn.close()


    def ReadAsDict(self, query, return_tup=False, convert_unicode=True):

        # ReadAsTuples already has a try-except block
        datatup,desc = self.ReadAsTuples(query)

        try:
            if len(datatup) == 0:
                return {}

            datadict = [] 
            irow = 0
            for row in datatup:
                datadict.append({})
                i=0
                for c in row:
                    if convert_unicode and isinstance(c, unicode):
                        val = str(c)
                    else:
                        val = c
                    datadict[irow][ desc[i][0] ] = val
                    i = i+1
                irow=irow+1

            if return_tup:
                return datadict, datatup, desc
            else:
                return datadict

        except:
            print 'error processing query results:',query.replace('\n','<br>'),'<p>'
            print traceback.format_exc().replace('\n','<br>')
            raise

    def ReadAsTuples(self, query):
        try:
            curs = self.conn.cursor()
            curs.execute(query)

            res = curs.fetchall()
            desc = curs.description
            curs.close()
            if len(res) < 1:
                return [], []
            return res, desc
        except:
            print 'error executing query:',query.replace('\n','<br>'),'<p>'
            print traceback.format_exc().replace('\n','<br>')
            raise
        
    def GetDescDict(self, desc):
        """
        Loop over the description tuple and create a dict lookup table between
        names and indices.  If only sqlite3 supported dict results
        """
        dict = {}
        i=0
        for d in desc:
            colname = d[0]
            dict[colname] = i
            i = i+1

        return dict

    def regexp(self, expr, item):
        """
        Return 1 if `item` matches the regular expression `expr`, 0 otherwise
        """
        r = re.compile(expr,re.IGNORECASE)
        return r.match(item) is not None




   


