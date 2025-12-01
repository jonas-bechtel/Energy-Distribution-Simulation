@echo off
echo "Running program in Debug"

set ROOTSYS=%~dp0\..\vendor\root_v6.32.04
set PATH=%~dp0\..\vendor\root_v6.32.04\bin;%PATH%
cd "%~dp0\..\Energy-Distribution-Model"
"%~dp0\..\bin\Debug-windows-x86_64\Energy-Distribution-Model\Energy-Distribution-Model.exe"

echo.
pause