#!/usr/bin/env python2.5

import sys
import os
import sqlite3
import sqlite_reader
import cgi

import recipe_util as ru

fs = cgi.FieldStorage()

print "Content-type: text/html\n\n"

ru.print_recipe_list(fs)

