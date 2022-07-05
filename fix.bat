@echo off

rem Sets makefile source code for the different platforms
rem Based on fix.bat of Allegro.
rem Modified By Kronoman - In loving memory of my father.

if [%1] == [djgpp]   goto djgpp
if [%1] == [mingw32] goto mingw32

goto help

:djgpp
echo Configuring for DOS/djgpp...
copy Makefile.dj Makefile
goto done

:mingw32
echo Configuring for Windows/Mingw32...
copy Makefile.m32 Makefile
goto done

:help
echo Usage: fix platform
echo.
echo Where platform is one of: djgpp, mingw32. 
echo.
goto end

:done
echo Done!

:end