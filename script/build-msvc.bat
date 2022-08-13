@echo off
if not '%vcscript%'=='' goto HaveBuildTools
for /f "delims=" %%i in ('dir /s /b "C:\Program Files (x86)\Microsoft Visual Studio\*vcvars64.bat"') do set vcscript="%%i"
if %vcscript%=="" echo Could not find VC BuildTools. Have you installed Visual Studio? & exit /b
call %vcscript%
:HaveBuildTools

cmake	-G Ninja ^
	-DCMAKE_MAKE_PROGRAM=ninja.exe ^
	-DCMAKE_BUILD_TYPE=RELEASE ^
	-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded" ^
	-B build/msvc .

cmake 	--build build/msvc