chcp 1251 
set robosd=I:\SOURCETREE\robosd++\content\

set JSON=I:\SOURCETREE\jsonsl
set ARMA=%robosd%\exlib\armadilio
set INILIB=%robosd%\exlib\miniini

set settings=%COMPUTERNAME%
set settings=%settings::=,%
set settings=%settings:.=,%
set settings=settings\%settings%
set BUILD=%robosd%\build

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

IF EXIST ".\reference\build-ref"  RMDIR ".\reference\build-ref"
MKLINK /J ".\reference\build-ref"  "%BUILD%"  

IF EXIST ".\reference\arma-ref"  RMDIR ".\reference\arma-ref"
MKLINK /J ".\reference\arma-ref"  "%ARMA%"  

IF EXIST ".\reference\inlib-ref"  RMDIR ".\reference\inlib-ref"
MKLINK /J ".\reference\inlib-ref"  "%INILIB%"


cd "%settings%"
echo [LOCAL] > robosd-pp.ini
echo CATALOG=%CD%\ >> robosd-pp.ini


cd %prev%
