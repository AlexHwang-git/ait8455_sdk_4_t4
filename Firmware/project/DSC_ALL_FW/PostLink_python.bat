set PRJ_NAME=ALL_FW
set AXF_DIR=.\%PRJ_NAME%_Data\PYTHON
set AXF_NAME=%PRJ_NAME%.axf
set RAM_DIR=%AXF_DIR%
set PKG_DIR=c:\aitfw\82x
set PKG_NAME=ALL
set H_DIR=c:\aitfw\82x

fromELF.exe -c -bin %AXF_DIR%\%AXF_NAME% -output %RAM_DIR%
:: ..\FirmwarePackager .\ldsfile_python.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q
..\FirmwarePackager .\ldsfile_python.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q /h %H_DIR%\%PKG_NAME%.h
