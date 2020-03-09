@echo off

path %ProgramFiles%\CMake\bin;%PATH%

if not exist build md build
cd build
cmake .. -G "Visual Studio 15 2017 Win64"
cd ..

pause
if %ERRORLEVEL% GEQ 1 EXIT /B 1
