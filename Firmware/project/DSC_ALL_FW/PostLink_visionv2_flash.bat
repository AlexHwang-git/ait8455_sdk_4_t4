set PRJ_NAME=ALL_FW
set AXF_DIR=.\%PRJ_NAME%_Data\VisionV2_Flash
set AXF_NAME=%PRJ_NAME%.axf
set RAM_DIR=%AXF_DIR%
set PKG_DIR=%AXF_DIR%
set PKG_NAME=VSN_V2FW
set H_DIR=%AXF_DIR%

::copy c:\aitfw\82x\VSN_V2BC %PKG_DIR%\VSN_V2BC 
fromELF.exe -c -bin %AXF_DIR%\%AXF_NAME% -output %RAM_DIR%\ALL_DRAM
..\FirmwarePackager .\ldsfile_visionv2_flash.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q

:: Generate Vision flash image 
..\VisionFlashImageGen 2 %PKG_DIR%\VSN_V2FlashImage.bin %PKG_DIR%\VSN_V2BC %PKG_DIR%\%PKG_NAME% VSN_V2

:: old FW packager
::..\MergeFirmwarePackage 2 %PKG_DIR%\VSN_V2FWImage.bin %PKG_DIR%\VSN_V2BC %PKG_DIR%\%PKG_NAME%
..\MergeFirmwarePackage 1 %PKG_DIR%\VSN_V2FWImage.bin %PKG_DIR%\VSN_V2FlashImage.bin
copy %PKG_DIR%\VSN_V2FWImage.bin c:\aitfw\82x\
copy %PKG_DIR%\VSN_V2BC c:\aitfw\82x\
copy %PKG_DIR%\VSN_V2FW c:\aitfw\82x\
copy ..\updater_vv2.bin c:\aitfw\82x\updater.bin

pause

