echo off

rem Visual Studio
del /Q .vs
rd /S /Q .vs
del /Q *.sln

rem Visual Studio Code
del /Q .vscode
rd /S /Q .vscode
del /Q *.code-workspace
del /Q .vsconfig

rem Rider
del /Q .idea
rd /S /Q .idea
del /Q .idea

rem Unreal Engine
del /Q Binaries
rd /S /Q Binaries

rem Build - build settings
del /Q "Build\WindowsNoEditor"
rd /S /Q "Build\WindowsNoEditor"
del /Q Build 
rd /S /Q Build 

rem Intermediate - temporary files generated when you compile your code
del /Q Intermediate
rd /S /Q Intermediate

:: rem Saved - local log & configuration files, screenshots, auto-saves etc.
:: del /Q Saved
:: rd /S /Q Saved

rem DerivedDataCache
del /Q DerivedDataCache
rd /S /Q DerivedDataCache

rem Script
del /Q Script
rd /S /Q Script

pause