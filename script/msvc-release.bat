@echo off
set _cwd=%CD%
set _script_dir=%~dp0
cd %_script_dir%
if not '%vcscript%'=='' goto HaveBuildTools
for /f "delims=" %%i in ('dir /s /b "C:\Program Files (x86)\Microsoft Visual Studio\*vcvars64.bat"') do set vcscript="%%i"
if %vcscript%=="" echo Could not find VC BuildTools. Have you installed Visual Studio? & cd %_cwd% & exit /b
call %vcscript%

:HaveBuildTools
call :normalize_path "../ext/bin/ninja.exe"
cmake	-G Ninja ^
	-DCMAKE_MAKE_PROGRAM=%retval% ^
        -DCMAKE_BUILD_TYPE=RELEASE ^
      	-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL" ^
	-B ../build/msvc/release ../.

set CL=/D_CRT_SECURE_NO_WARNINGS=1 %CL%
cmake 	--build ../build/msvc/release
cd %_cwd%
exit /b

:normalize_path
set retval=%~f1
exit /b