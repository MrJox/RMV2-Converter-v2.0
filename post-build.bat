@echo off

rem Check if the number of arguments passed is less than 2
if "%~2" == "" (
    echo Usage: %0 SourceFolder DestinationFolder
    exit /b
)

set src=%1
set dst=%2

xcopy "%src%" "%dst%" /E /I /Y
