cl /Fetestwin.exe /GX /GR /TP *.cc /link alleg.lib
del test.obj
lib /OUT:gstream.lib *.obj
del *.obj
makedoc gstream._tx
copy gmanip.h %MSVCDir%\include\
copy gmanip %MSVCDir%\include\
copy gstream.h %MSVCDir%\include\
copy gstream %MSVCDir%\include\
copy gstream.lib %MSVCDir%\lib\
