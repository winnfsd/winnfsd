@echo off
:: Note: You can add the /B flag to the exit commands to keep the command window open. 
:: Example:  exit /B 1

tasklist /nh /fi "imagename eq winnfsd.exe" 2>nul | find /I /C "winnfsd.exe" >nfsservicetmp
set /p RUNNINGTASKS=<nfsservicetmp
del nfsservicetmp

if %1==status (
    echo "[NFS] Status: "
    if %RUNNINGTASKS% == 0 (
        echo "halted\n"
        exit 1
    ) else (
        echo "running\n"
        exit 0
    )
)

if %1==start (
    echo "[NFS] Start: "
    if %RUNNINGTASKS% == 0 (
        start winnfsd -log off -pathFile %2
        echo "started\n"
    ) else (
        echo "already running\n"
    )
    
    exit 0
)

if %1==halt (
    echo "[NFS] Halt: "
    if %RUNNINGTASKS% == 0 (
        echo "not running\n"
    ) else (
        taskkill /f /im "winnfsd.exe" >nul
        echo "halt\n"
    )
    
    exit 0
)

exit 1