set PRJ_NAME=ALL_FW
set AXF_DIR=.\%PRJ_NAME%_Data\PYTHON_FLASH
set AXF_NAME=%PRJ_NAME%.axf
set RAM_DIR=%AXF_DIR%
set PKG_DIR=%AXF_DIR%
set PKG_NAME=PCCAM
set H_DIR=%AXF_DIR%
set ROMPKG_NAME=AIT842ROMPackage
set ROMPKG_NAME1=AIT842ROMPackage_BC8

fromELF.exe -c -bin %AXF_DIR%\%AXF_NAME% -output %RAM_DIR%
:: ..\FirmwarePackager .\ldsfile_python.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q
..\FirmwarePackager .\ldsfile_python_flash.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q /h %H_DIR%\%PKG_NAME%.h

;;:: old FW packager
;;..\MergeFirmwarePackage 2 %PKG_DIR%\842PccamFlashImage.bin %PKG_DIR%\A842_BC2 %PKG_DIR%\%PKG_NAME%
;;copy %PKG_DIR%\842PccamFlashImage.bin c:\aitfw\82x\

..\MergeFirmwarePackage 2 %PKG_DIR%\842PccamFlashImage_BC8.bin %PKG_DIR%\A842_BC8 %PKG_DIR%\%PKG_NAME%
copy %PKG_DIR%\842PccamFlashImage_BC8.bin c:\aitfw\82x\

:: new ROM packager
..\ROMPackager %PKG_DIR%\RomPackageDesc.ini %PKG_DIR%\%ROMPKG_NAME%.rom
copy %PKG_DIR%\%ROMPKG_NAME%.rom c:\aitfw\82x\

..\ROMPackager %PKG_DIR%\RomPackageDescBC8.ini %PKG_DIR%\%ROMPKG_NAME1%.rom
copy %PKG_DIR%\%ROMPKG_NAME1%.rom c:\aitfw\82x\

..\merge %PKG_DIR%\updater_boot.bin %PKG_DIR%\842PccamFlashImage_BC8.bin %PKG_DIR%\updater.bin
copy  %PKG_DIR%\updater.bin c:\aitfw\82x\

..\bin2flash 0 %PKG_DIR%\842PccamFlashImage_BC8.bin %PKG_DIR%\842PccamFlash256k.flash
..\bin2flash 1 %PKG_DIR%\842PccamFlashImage_BC8.bin %PKG_DIR%\842PccamFlash256k.dfu

copy  %PKG_DIR%\842PccamFlash256k.flash c:\aitfw\82x\
copy  %PKG_DIR%\842PccamFlash256k.dfu c:\aitfw\82x\

fromelf -c %PKG_DIR%\ALL_FW.axf > c:\aitfw\82x\1.s
 

