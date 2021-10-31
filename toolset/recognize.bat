@echo off
rem 600:160:440:200

set src=%1
set mask=%2
set pos=%3

if "%src%" == "" goto USAGE
if "%mask%" == "" goto USAGE
if "%pos%" == "" goto USAGE

echo Processing pattern file...
kpextract -i "%src%" -s 8192:8192 >"%src%.kp"
for %%i in (%mask%) do call :BODY "%%i"
goto :EOF

rem ---------------------------------------------------------------------------
:BODY
echo Processing %1...

kpmatch -i "%1" -k "%src%.kp" -o "%1.jpg" -s 1024:1024
if errorlevel 1 goto :EOF

set code=%1
for /F "Tokens=*" %%i in ('rec2of5 -i "%1.jpg" -c -p %pos%') do set code=%%i
if errorlevel 1 goto :EOF

move "%1.jpg" "%code%.jpg"
goto :EOF
rem ---------------------------------------------------------------------------


:USAGE
echo recognize - barcode recognizing script.
echo.
echo USAGE: 
echo   recognize pattern-file file-mask barcode-position
echo.
echo Barcode position in pattern file must be given in a form x:y:w:h.
goto :EOF
