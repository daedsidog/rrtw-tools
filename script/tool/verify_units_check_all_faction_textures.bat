@echo off
cd bin
verificator.exe ../../../RIS --check-all-faction-textures --check-all-referenced-textures
cd ..
pause