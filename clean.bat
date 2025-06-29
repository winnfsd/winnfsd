@echo off
cd /D %~dp0

set PROJECT_TAR=x64

rmdir /q /s .vs

if exist output rmdir /q /s output

for %%N in (%PROJECT_TAR%) do ( 
	for /f %%D in ('dir %%N /b /s') do ( 
		if exist "%%D" rmdir /q /s %%D
	)
) 2> nul