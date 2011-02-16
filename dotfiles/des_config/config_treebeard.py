import os


# file descriptions
files = {}

rcname = 'red_cat'
files[rcname] = {}
files[rcname]['type'] = rcname
files[rcname]['format'] = 'fits'
files[rcname]['data_hdu'] = 2
files[rcname]['data_type'] = 'bintable'
files[rcname]['meta_hdu'] = 2
files[rcname]['meta_type'] = 'header'
files[rcname]['table'] = 'cat'
files[rcname]['meta_table'] = 'cat_meta'




# table descriptions
tables = {}

tdef = \
    {'table_name':None, 
     'desc':None,
     'unique_cols':None,
     'serial_cols':None,
     'indexes':None}


ftable = 'files'
path_sdef = 'S125'
tables[ftable] = tdef
tables[ftable]['table_name'] = ftable
tables[ftable]['desc'] = \
    [('type','S10'),
     ('format','S10'),
     ('relpath',path_sdef),
     ('desc',path_sdef),
     ('basename','S50'),
     ('dirname',path_sdef)]

# note these automatically get indexes
tables[ftable]['unique_cols'] = ['fileid','filedesc','filepath']
tables[ftable]['serial_cols'] = ['fileid']



