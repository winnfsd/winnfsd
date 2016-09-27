# WinNFSd

[![Build status](https://ci.appveyor.com/api/projects/status/github/winnfsd/winnfsd?svg=true)](https://ci.appveyor.com/project/MarcHarding/winnfsd-6y1xi/branch/master)

Introduction
------------
* Fork of WinNFSd_edited by ZeWarden(http://github.com/ZeWaren/WinNFSd_edited), based on WinNFSd by vincentgao (http://sourceforge.net/projects/winnfsd/).
* License: GPL.
* Runs on all major versions of Windows.

Description
--------------------
WinNFSd is a Network File System V3 (NFS) server for Windows.

You can use any NFS client to mount a directory of Windows and read/write files via NFS v3 protocol. It is useful when you usually access files of Windows on Linux and for especially for virtual machines, since it is much faster than shared folders.

You can also export any folder with an additional alias.

The export of multiple folders is also possible. Just put the shared foldes and an optional alias into a simple text file:

```
C:\path\to\a\mount > /alias
C:\path\to\another\mount > /another-alias
```

Then start winnfsd.exe like this:
`WinNFSd.exe -pathFile C:\path\to\your\pathfile`


Usage
-------------------
```
=====================================================
WinNFSd 2.2.0
Network File System server for Windows
Copyright (C) 2005 Ming-Yang Kao
Edited in 2011 by ZeWaren
Edited in 2013 by Alexander Schneider (Jankowfsky AG)
Edited in 2014 2015 by Yann Schepens
Edited in 2016 by Peter Philipp (Cando Image GmbH), Marc Harding
=====================================================

Usage: WinNFSd.exe [-id <uid> <gid>] [-log on | off] [-pathFile <file>] [-addr <ip>] [export path] [alias path]

At least a file or a path is needed
For example:
On Windows> WinNFSd.exe d:\work
On Linux> mount -t nfs 192.168.12.34:/d/work mount

For another example:
On Windows> WinNFSd.exe d:\work /exports
On Linux> mount -t nfs 192.168.12.34:/exports

Another example where WinNFSd is only bound to a specific interface:
On Windows> WinNFSd.exe -addr 192.168.12.34 d:\work /exports
On Linux> mount - t nfs 192.168.12.34: / exports

Use "." to export the current directory (works also for -filePath):
On Windows> WinNFSd.exe . /exports
```
