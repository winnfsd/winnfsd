WinNFSd
=======

Introduction
------------
* Fork of WinNFSd_edited by ZeWarden(http://github.com/ZeWaren/WinNFSd_edited), based on WinNFSd by vincentgao (http://sourceforge.net/projects/winnfsd/).
* License: GPL.
* Runs on all major versions of Windows.

Original Description
--------------------
WinNFSd is a Network File System (NFS) server for Windows. You can use any NFS client to mount a directory of Windows and read/write files via NFS protocol. It is useful when you usually access files of Windows on Linux.
Can export any folder with an alias. This can be very useful if you quickly need to share a directory using NFS on a Windows computer, without installing anything.
Example: export `c:\truc\machin` as `/`.

Additional features
-------------------
Posibillity to export more than one folder by using a simple text file.

Usage
-------------------
	=====================================================
    WinNFSd v2.0
    Network File System server for Windows
    Copyright (C) 2005 Ming-Yang Kao
    Edited in 2011 by ZeWaren
    Edited in 2013 by Alexander Schneider (Jankowfsky AG)
    =====================================================

    Usage: WinNFSd.exe [-id <uid> <gid>] [-log on | off] [-pathFile <file>] [export path] [alias path]

    At least a file or a path is needed
    For example:
    On Windows> WinNFSd.exe d:\work
    On Linux> mount -t nfs 192.168.12.34:/d/work mount

    For another example:
    On Windows> WinNFSd.exe d:\work /exports
    On Linux> mount -t nfs 192.168.12.34:/exports

    Use "." to export the current directory (works also for -filePath):
    On Windows> WinNFSd.exe . /exports