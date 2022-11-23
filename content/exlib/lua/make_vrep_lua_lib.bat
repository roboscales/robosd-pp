DEL lua.lib;
echo LIBRARY LUA > lua.def
echo EXPORTS >> lua.def
dumpbin /exports lua.dll >> luadump.txt
FOR /F "skip=19 tokens=4" %i in (luadump.txt) DO @echo %i >> lua.def
@ECHO cla lib

lib /def:lua.def /out:lua.lib /machine:x86

DEL lua.def
DEL luadump.txt
