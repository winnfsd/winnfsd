@echo off
set UID="1000"
set GID="1000"
set EXPORTS="c:\winnfsd\exports.txt"

title "NFS server for windows, uid %UID% %GID%, exports %EXPORTS%"

c:\winnfsd\winnfsd.exe -log off -uid %UID% %GID% -pathFile %EXPORTS%
