#!/usr/bin/env python
import wx
import os

class MainWindow(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self, parent, title=title, size=(200,100))

        self.control = wx.TextCtrl(self, style=wx.TE_MULTILINE)
        self.CreateStatusBar() # A Statusbar in the bottom of the window

        # menu begins not connected to any frame 
        filemenu= wx.Menu()


        about_menuitem = \
            filemenu.Append(wx.ID_ABOUT, "&About"," Information about this program")

        filemenu.AppendSeparator()

        open_menuitem = \
            filemenu.Append(wx.ID_OPEN, "&Open"," Open a file to edit")

        filemenu.AppendSeparator()

        exit_menuitem = \
            filemenu.Append(wx.ID_EXIT,"E&xit"," Terminate the program")

        # make it so the about menu item will do something
        self.Bind(wx.EVT_MENU, self.OnAbout, about_menuitem)
        self.Bind(wx.EVT_MENU, self.OnExit, exit_menuitem)
        self.Bind(wx.EVT_MENU, self.OnOpen, open_menuitem)

        # Creating the menubar.
        menuBar = wx.MenuBar()
        menuBar.Append(filemenu,"&File") # Adding the "filemenu" to the MenuBar

        self.SetMenuBar(menuBar)  # Adding the MenuBar to the Frame content.


        self.sizer2 = wx.BoxSizer(wx.HORIZONTAL)
        self.buttons = []
        for i in range(0, 6):
            self.buttons.append(wx.Button(self, -1, "Button &"+str(i)))
            self.sizer2.Add(self.buttons[i], 1, wx.EXPAND)

        # Use some sizers to see layout options
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        # second par is "proportion".  I think 0 means "as small as possible"
        # and 1 means "as big as possible".  If both were 1 they would each
        # take up half
        self.sizer.Add(self.sizer2, 0, wx.EXPAND)
        self.sizer.Add(self.control, 1, wx.EXPAND)

        #Layout sizers
        self.SetSizer(self.sizer)
        self.SetAutoLayout(1)
        self.sizer.Fit(self)



        self.Show(True)

    def OnOpen(self,e):
        """ Open a file"""
        self.dirname = ''
        dlg = wx.FileDialog(self, "Choose a file", self.dirname, "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            self.filename = dlg.GetFilename()
            self.dirname = dlg.GetDirectory()
            f = open(os.path.join(self.dirname, self.filename), 'r')
            self.control.SetValue(f.read())
            f.close()
        dlg.Destroy()

    def OnAbout(self,e):
        # A message dialog box with an OK button. 
        # wx.OK is a standard ID in wxWidgets.
        dlg = wx.MessageDialog(self, 
                               "A small text editor",  # message to show
                               "About Sample Editor",  # title of box
                               wx.OK)                  # standard button builtin
        dlg.ShowModal() # Show it
        dlg.Destroy() # finally destroy it when finished.

    def OnExit(self,e):
        self.Close(True)  # Close the frame.


app = wx.App(False)
frame = MainWindow(None, "Sample editor")
app.MainLoop()

