chcp 1251 
set robosd=I:\SOURCETREE\robosd++\content\

set JSON=I:\SOURCETREE\jsonsl

set settings=%COMPUTERNAME%
set settings=%settings::=,%
set settings=%settings:.=,%
set settings=settings\%settings%

set prev=%CD%

cd /D "%~dp0"

if not EXIST reference (
	MD   reference 
)


if not EXIST settings (
	MD   settings 
)


if not EXIST "%settings%" (
	MD   "%settings%" 
)


IF EXIST ".\reference\settings-ref"  RMDIR ".\reference\settings-ref"
MKLINK /J ".\reference\settings-ref"  "%settings%"  

IF EXIST ".\reference\robosd-ref"  RMDIR ".\reference\robosd-ref"
MKLINK /J ".\reference\robosd-ref"  "%robosd%"\source  

IF EXIST ".\reference\json-ref"  RMDIR ".\reference\json-ref"
MKLINK /J ".\reference\json-ref"  "%JSON%"  

cd "%settings%"
echo [LOCAL] > robosd-pp.ini
echo CATALOG=%CD%\ >> robosd-pp.ini


cd %prev%
