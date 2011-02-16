#!/usr/bin/env python
import wx

app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
frame = wx.Frame(None, wx.ID_ANY, "Hello World") # A Frame is a top-level window.
frame.Show(True)     # Show the frame.
frame2 = wx.Frame(None, wx.ID_ANY, "And Stuff")
frame2.Show(True)

frame2sub = wx.Frame(frame2, wx.ID_ANY, "Blah")
frame2sub.Show(True)
app.MainLoop()

