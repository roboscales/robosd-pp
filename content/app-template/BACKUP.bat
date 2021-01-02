@echo off

set ttt=%CD%



set robosd=robosd_rev_6(pp)

md %TMP%\~%robosd%

cd /D "%~dp0"

xcopy *.* "%TMP%\~%robosd%" /S /Y /D

cd /D %TMP%\~%robosd%
echo %CD%
CALL CLEAN

SET hs=%COMPUTERNAME%
SET fn=%robosd%_%hs%_%DATE%_%TIME%
SET fn=%fn::=,%
SET fn=%fn:.=,%


zip -r "%fn%.zip"  *.bat zip.exe  core settings  reference


cd /D "%ttt%\..\..\"


copy "%TMP%\~%robosd%\%fn%.zip 

cd /D "%ttt%"

rd "%TMP%\~%robosd%" /S /Q



