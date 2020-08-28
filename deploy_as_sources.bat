::
:: Deploys sources of the dscpp library to a given
:: directory. Will create a subdirectory for the library.
:: Example: > .\deploy_as_sources.bat C:\libs
::
@echo off
setlocal

set outDir=%1
set thisDir=%~dp0

set src=%thisDir%
set dest=%outDir%\dscpp

:: Deploy code.
mkdir %dest%
robocopy %src% %dest% *.h
robocopy %src% %dest% *.cpp
