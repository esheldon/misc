#!/usr/bin/env python
"""
View a list of files as a grid.  Each will be clickable to show
it's contents
"""
import wx
import wx.html
import wx.grid
import os
import glob

import email
import webbrowser

import re

configurables="""
text wrap width

colors for each header element?
colors for each header value?

text colors overall
background overall

quotation levels
"""


class MyHtmlWindow(wx.html.HtmlWindow):
    def __init__(self, *args, **kwargs):
        wx.html.HtmlWindow.__init__(self, *args, **kwargs)
    def OnLinkClicked(self, link_info):
        webbrowser.open(link_info.GetHref())



ID_SPLITTER=300
class BrowserWindow(wx.Frame):
    def __init__(self, parent, title, **keys):

        self.full_header = keys.get('full_header',False)
        self.header_keys = keys.get('headers',None)
        if self.header_keys is None:
            self.header_keys = ['From',
                                'To',
                                'Reply-To',
                                'Cc',
                                'Bcc',
                                'Date',
                                'Message-Id',
                                'User-Agent',
                                'Subject']

        self.email_text_ctrl = keys.get('email_text_ctrl',False)

        self.url_finder = UrlFinder()


        wx.Frame.__init__(self, parent, title=title)

        self.splitter = wx.SplitterWindow(self, ID_SPLITTER, style=wx.SP_BORDER)

        self.dir = keys.get('dir','')
        self.CreateStatusBar() # A Statusbar in the bottom of the window

        # this is the file listing
        self.fill_elist_ctrl()

        # this is where we will put the text of the file when
        # a file is selected
        if self.email_text_ctrl:
            self.email_text = wx.TextCtrl(self.splitter, -1, 
                                          style=wx.TE_MULTILINE | wx.TE_READONLY)
            self.splitter.SplitHorizontally(self.elist_ctrl, self.email_text)
        else:
            self.email_header_panel = wx.Panel(self.splitter)
            self.email_header_sizer = wx.BoxSizer(wx.VERTICAL)

            self.header_html = MyHtmlWindow(self.email_header_panel)
            self.email_html= MyHtmlWindow(self.email_header_panel)

            self.email_header_sizer.Add(self.header_html, proportion=1, flag=wx.EXPAND)
            self.email_header_sizer.Add(self.email_html, proportion=4, flag=wx.EXPAND)
            #self.email_header_sizer.Add(self.header_html, flag=wx.EXPAND)
            #self.email_header_sizer.Add(self.email_html, flag=wx.EXPAND)

            self.email_header_panel.SetSizer(self.email_header_sizer)

            self.splitter.SplitHorizontally(self.elist_ctrl, 
                                            self.email_header_panel)


        self.main_sizer = wx.BoxSizer(wx.VERTICAL)

        #self.main_sizer.Add(self.elist_ctrl, 1, wx.EXPAND)
        self.main_sizer.Add(self.splitter, 1, wx.EXPAND)

        self.SetSizer(self.main_sizer)
        self.SetAutoLayout(1)
        self.main_sizer.Fit(self)



        self.Show(True)

    def fill_elist_ctrl(self):
        self.email_files = self.get_email_list(self.dir)
        nemail = len(self.email_files)
        self.elist_ctrl = wx.ListCtrl(self.splitter, -1, style=wx.LC_REPORT | wx.LC_HRULES | wx.LC_VRULES, size=(-1,200))

        self.elist_ctrl.Bind(wx.EVT_LIST_ITEM_SELECTED, self.email_selected)

        self.elist_ctrl.InsertColumn(0, "Date")
        #self.elist_ctrl.InsertColumn(1, "From", wx.LIST_FORMAT_RIGHT)
        self.elist_ctrl.InsertColumn(1, "From")
        self.elist_ctrl.InsertColumn(2, "Subject")
        #self.elist_ctrl.SetColumnWidth(0, 350)
        #self.elist_ctrl.SetColumnWidth(1, 70)


        row=0
        for fname in self.email_files:
            msg = self.get_message_from_file(fname)
            datestr = msg['Date']
            fromstr = unicode(msg['From'],'utf-8',errors='replace')
            fromtuple = email.utils.parseaddr(fromstr)
            if fromtuple[0] == '':
                fromstr = fromtuple[1]
            else:
                fromstr = fromtuple[0]
            subject=msg['Subject']
            if subject is None:
                subject = ''
            else:
                subject = unicode(subject,'utf-8',errors='replace')
            self.elist_ctrl.InsertStringItem(row, datestr)
            self.elist_ctrl.SetStringItem(row, 1, fromstr)
            self.elist_ctrl.SetStringItem(row, 2, subject)

            if (row % 2) == 0:
                self.elist_ctrl.SetItemBackgroundColour(row, '#e6f1f5')

            row+=1

        self.elist_ctrl.SetColumnWidth(0, wx.LIST_AUTOSIZE) 
        self.elist_ctrl.SetColumnWidth(1, 200)
        self.elist_ctrl.SetColumnWidth(2, wx.LIST_AUTOSIZE) 

    def email_selected(self, event):
        i = event.GetIndex() # Find item selected
        print 'Selected row:',i 
        msg = self.get_message_from_file(self.email_files[i])

        if self.email_text_ctrl:
            self.show_email_as_text_ctrl(msg)
        else:

            parts = self.get_message_parts(msg)
            self.show_header_as_html(parts['header'])
            if len(parts['body']['text/plain']) != 0:
                self.show_text_email_as_html(parts)
            elif len(parts['body']['text/html']) != 0:
                self.show_html_email_as_html(parts)
            else:
                raise ValueError("No text/plain or text/html found")

    def get_email_list(self, dir):
        return glob.glob(dir+'/*')

    def show_email_as_text_ctrl(self, msg):
        self.email_text.Clear()

        header_list = self.extract_header_list(msg, use_tuple=False)
        parts_list = self.extract_parts_list(msg)

        all_list = header_list + parts_list
        email_text = '\n'.join(all_list)
        try:
            self.email_text.AppendText(email_text)
        except:
            self.email_text.AppendText(unicode(email_text,'utf-8',errors='replace'))
        self.email_text.ShowPosition(0)


    def show_email_as_text(self, msg):
        #self.email_text.Clear()

        header_list = self.extract_header_list(msg, use_tuple=False)
        parts_list = self.extract_parts_list(msg)

        all_list = header_list + parts_list
        email_text = '\n'.join(all_list)
        email_text = """
        <html>
        <!-- <body bgcolor="#1f1f1f" text="#dcdccc"> -->
        <body>
<pre>{email_text}</pre>
        </body>
        </html>
        """.format(email_text=email_text)
        self.email_html.SetPage(email_text)
        #self.email_text.AppendText(email_text)
        #self.email_text.ShowPosition(0)

    def show_text_email_as_html(self, parts):
        """
        Make a styled header and a body.  The body will be the concatenation
        of all 'text/plain' parts.  Newlines are converted to html line
        breaks.  Url's are converted to <a href=....>

        The real purpose of this is to give clickable links to the body, which
        cannot be done in the text viewer as far as I know.  We could use a
        separate window above the body for the stylized header if needed.

        """

        parts_list = []
        for tmsg in parts['body']['text/plain']:
            try:
                tmp = tmsg.get_payload(decode=True)
            except:
                tmp = tmsg.get_payload(decode=False)
            tmp = tmp.replace('\n','<br>')
            tmp = self.url_finder.sub_url_with_link(tmp)
            parts_list.append(tmp)
        email_text = '\n'.join(parts_list)

        email_text = """
        <html>
        <!-- <body bgcolor="#1f1f1f" text="#dcdccc"> -->
        <tt>
        <body>
            {email_text}
        </body>
        </tt>
        </html>
        """.format(email_text=email_text)

        try:
            self.email_html.SetPage(email_text)
        except:
            self.email_html.SetPage(unicode(email_text,'utf-8',errors='replace'))

    def show_html_email_as_html(self, parts):
        """
        Make a styled header and the html body as is.

        """

        hrows = []
        for h in parts['header']:
            print h
            key = h[0]
            val = h[1].replace('<', '&lt;').replace('>','&gt;')
            rowhtml = '<tr><td valign=top><b>' + key +':</b> &nbsp;</td><td valign=top>'+ val +'</td></tr>'
            print rowhtml
            hrows.append(rowhtml)

        hrows = '\n'.join(hrows)

        parts_list = []
        for tmsg in parts['body']['text/html']:
            try:
                tmp = tmsg.get_payload(decode=True)
            except:
                tmp = tmsg.get_payload(decode=False)
            tmp = tmp.replace('\n','<br>')
            tmp = self.url_finder.sub_url_with_link(tmp)
            parts_list.append(tmp)
        email_text = '\n'.join(parts_list)

        email_text = """
        <html>
        <!-- <body bgcolor="#1f1f1f" text="#dcdccc"> -->
        <tt>
        <body>
            <table border=0>
                <tr>
                    <td>
                        <table bgcolor=white border=0 cellspacing=0 cellpadding=0>
                        {hrows}
                        </table>
                    </td>
                </tr>
                <tr>
                    <td>
                        {email_text}
                    </td>
                </tr>
            </table>
        </body>
        </tt>
        </html>
        """.format(hrows=hrows, email_text=email_text)

        try:
            self.email_html.SetPage(email_text)
        except:
            self.email_html.SetPage(unicode(email_text,'utf-8',errors='replace'))





    # read in the email message from an arg or stdin
    def get_message_from_file(self, name):
        fp = open(name)
        return email.message_from_file(fp)

    def extract_header_list(self, msg, use_tuple=True):
        tlist = []
        if self.full_header:
            for key in msg.keys():
                val = msg[key]
                if use_tuple:
                    tlist.append((key,str(val)))
                else:
                    tlist.append('%s: %s' % (key,val))
        else:
            for key in self.header_keys:
                if key in msg:
                    val = msg[key]
                    if use_tuple:
                        tlist.append((key,str(val)))
                    else:
                        tlist.append('%s: %s' % (key,val))

        if not use_tuple:
            tlist.append("")# one extra newline in case text abuts headers
        return tlist
 
    def get_message_parts(self, msg):
        parts = {}
        parts['header'] = self.extract_header_list(msg)
        # body could be 'text/plain' or 'text/html'
        parts['body'] = {}
        parts['body']['text/plain'] = []
        parts['body']['text/html'] = []
        parts['attachments'] = []

        for tmsg in msg.walk():
            f = tmsg.get_filename()
            if f is not None:
                parts['attachments'].append(tmsg)
            else:
                type = tmsg.get_content_type()
                if type in ['text/plain','text/html']:
                    parts['body'][type].append(tmsg)
                else:
                    parts['body'][type] = tmsg
        return parts


    def extract_parts_list(self, msg):
        tlist = []
        for part in msg.walk():
            t=self.extract_part(part)
            if t is not None:
                tlist.append(t)
        return tlist

    def extract_part(self, part):
        type = part.get_content_type()
        if part.get_filename() is not None:
            return "# Attachment: %s (%s)\n" % (part.get_filename(),type)
        elif type == 'text/plain':
            try:
                payload = str( part.get_payload(decode=True) )
            except:
                payload = str( part.get_payload(decode=False) )
            return payload
        else:
            # might be html
            return None
        stuff="""
        elif type == "text/plain" or type == "text/html":
            command = "w3m -dump -T %s" % type
            try:
                ch = Popen(command,shell=True,stdin=PIPE,stdout=PIPE,stderr=PIPE)
            except OSError as e:
                stderr.write("Execution failed: %s\n" % e)
                sys.exit()
            ch.stdin.write(str(part.get_payload(decode=True)))
            ch.stdin.close()
            out = ch.stdout.read().strip()
            err = ch.stderr.read()
            ch.wait()
            if len(err):
                stdout.write("Error with w3m: %s\n" % err)
                sys.exit()
            return out
        """

    def show_header_as_html(self, header_list):
        hrows = []
        for h in header_list:
            #print h
            key = h[0]
            val = h[1].replace('<', '&lt;').replace('>','&gt;')
            rowhtml = '<tr><td valign=top><b>' + key +':</b> &nbsp;</td><td valign=top>'+ val +'</td></tr>'
            print rowhtml
            hrows.append(rowhtml)

        hrows = '\n'.join(hrows)
 
        header_text = """
        <html>
        <!-- <body bgcolor="#1f1f1f" text="#dcdccc"> -->
        <tt>
        <body>

            <table bgcolor=white border=0 cellspacing=0 cellpadding=0>
            {hrows}
            </table>
        </body>
        </tt>
        </html>
        """.format(hrows=hrows)

        try:
            self.header_html.SetPage(header_text)
        except:
            self.header_html.SetPage(unicode(header_text,'utf-8',errors='replace'))


    def show_text_email_as_html_old(self, msg):
        import textwrap
        header_list = self.extract_header_list(msg, use_tuple=True)
        parts_list = self.extract_parts_list(msg)

        hrows = []
        for h in header_list:
            #print h
            key = h[0]
            val = h[1].replace('<', '&lt;').replace('>','&gt;')
            rowhtml = '<tr><td valign=top><b>' + key +':</b> &nbsp;</td><td valign=top>'+ val +'</td></tr>'
            print rowhtml
            hrows.append(rowhtml)

        hrows = '\n'.join(hrows)
        #all_list = header_list + parts_list

        for i in xrange(len(parts_list)):
            parts_list[i] = parts_list[i].replace('\n','<br>')
            parts_list[i] = self.url_finder.sub_url_with_link(parts_list[i])
            #print self.url_finder.get_url_list(parts_list[i])
        #email_text = '\n'.join(all_list)
        email_text = '\n'.join(parts_list)
        """
        email_text =textwrap.fill(email_text,120,
                                  drop_whitespace=False,
                                  replace_whitespace=False,
                                  break_long_words=False,
                                  expand_tabs=False)
        """
        email_text = """
        <html>
        <!-- <body bgcolor="#1f1f1f" text="#dcdccc"> -->
        <tt>
        <body>
            <table border=0>
                <tr>
                    <td>
                        <table bgcolor=white border=0 cellspacing=0 cellpadding=0>
                        {hrows}
                        </table>
                    </td>
                </tr>
                <tr>
                    <td>
                        {email_text}
                    </td>
                </tr>
            </table>
        </body>
        </tt>
        </html>
        """.format(hrows=hrows, email_text=email_text)

        try:
            self.email_html.SetPage(email_text)
        except:
            self.email_html.SetPage(unicode(email_text,'utf-8',errors='replace'))



class UrlFinder:
    def __init__(self):


        urls = '(%s)' % '|'.join("""http https file ftp""".split())
        ltrs = r'\w'
        gunk = r'/#~:.?+=&%@!\-'
        punc = r'.:?\-'
        any = "%(ltrs)s%(gunk)s%(punc)s" % { 'ltrs' : ltrs,
                                             'gunk' : gunk,
                                             'punc' : punc }

        url = r"""
            \b                            # start at word boundary
            (                             # begin \1 {
                %(urls)s    :             # need resource and a colon
                [%(any)s] +?              # followed by one or more
                                          #  of any valid character, but
                                          #  be conservative and take only
                                          #  what you need to....
            )                             # end   \1 }
            (?=                           # look-ahead non-consumptive assertion
                    [%(punc)s]*           # either 0 or more punctuation
                    [^%(any)s]            #  followed by a non-url char
                |                         # or else
                    $                     #  then end of the string
            )
            """ % {'urls' : urls,
                   'any' : any,
                   'punc' : punc }

        
        self.url_re = re.compile(url, re.VERBOSE | re.MULTILINE | re.IGNORECASE)

    def get_url_list(self, text):
        return self.url_re.findall(text)
    def sub_url_with_link(self, text):
        """
        For every url, replace it with <a href="url">url</a>
        """
        #return self.url_re.sub(r'<a href="\1">\1</a>', text)
        return self.url_re.sub(r'<a href="\g<1>">\g<1></a>', text)





app = wx.App(False)
frame = BrowserWindow(None, "Email List", email_text_ctrl=False, dir='./emails')
app.MainLoop()

