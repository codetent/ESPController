@echo off

if "%1" == "" (
    echo "Path to IDF not given"
) else (
    set IDF_PATH=%1
    call %1/export.bat
)
