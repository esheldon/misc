#!/usr/bin/env python2.5

import cgi
import os
import re
import sqlite3 as sqlite
import recipe_util as ru

print "Content-type: text/html\n\n"
ru.print_recipe_book_stats()
