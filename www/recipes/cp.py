#!/usr/bin/env python2.5

import cgi
import shutil
import os


print "Content-type: text/html\n\n"
print

dbfiles=['Danielle_and_Johns_Recipes.db',
         'Erins_Recipes.db',
         'My_moms_and_my_favourites.db',
         'Sarahs_Recipes.db',
         'Spielford_Succotash.db',
         'cathy_recipes.db']

indir='old_db'
outdir='data'

for db in dbfiles:
    fold=os.path.join(indir, db)
    fnew=os.path.join(outdir, db)
    shutil.copyfile(fold, fnew)


print os.listdir(outdir)


