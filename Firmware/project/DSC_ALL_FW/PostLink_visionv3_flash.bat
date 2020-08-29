set PRJ_NAME=ALL_FW
set AXF_DIR=.\%PRJ_NAME%_Data\VisionV3_Flash
set AXF_NAME=%PRJ_NAME%.axf
set RAM_DIR=%AXF_DIR%
set PKG_DIR=%AXF_DIR%
set PKG_NAME=VSN_V3FW
set H_DIR=%AXF_DIR%

::copy c:\aitfw\82x\VSN_V2BC %PKG_DIR%\VSN_V3BC 
fromELF.exe -c -bin %AXF_DIR%\%AXF_NAME% -output %RAM_DIR%\ALL_DRAM
:: fromELF.exe -c  %AXF_DIR%\%AXF_NAME% > c:\1.s
..\FirmwarePackager .\ldsfile_visionv3_flash.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q

:: Generate Vision flash image 
::format: VisionFlashImageGen.exe ChipID outfile infileNum infile1 enable1 infile2 enable2...
..\VisionFlashImageGen VSN_V3 %PKG_DIR%\VSN_V3FlashImage.bin 2 %PKG_DIR%\VSN_V3BC 1 %PKG_DIR%\%PKG_NAME% 1 
..\VisionFlashImageGen VSN_V3 %PKG_DIR%\VSN_V3FlashImageFWOnly.bin 2 %PKG_DIR%\VSN_V3BC 0 %PKG_DIR%\%PKG_NAME% 1 

:: old FW packager
::..\MergeFirmwarePackage 2 %PKG_DIR%\VSN_V3FWImage.bin %PKG_DIR%\VSN_V3BC %PKG_DIR%\%PKG_NAME%
..\MergeFirmwarePackage 1 %PKG_DIR%\VSN_V3FWImage.bin %PKG_DIR%\VSN_V3FlashImage.bin
..\MergeFirmwarePackage 1 %PKG_DIR%\VSN_V3FWOnlyImage.bin %PKG_DIR%\VSN_V3FlashImageFWOnly.bin

copy %PKG_DIR%\VSN_V3FWImage.bin c:\aitfw\P_8455\
copy %PKG_DIR%\VSN_V3FWOnlyImage.bin c:\aitfw\P_8455\
copy %PKG_DIR%\VSN_V3FlashImage.bin c:\aitfw\P_8455\
copy %PKG_DIR%\VSN_V3BC c:\aitfw\P_8455\
copy %PKG_DIR%\VSN_V3FW c:\aitfw\P_8455\
copy ..\updater_vv3.bin c:\aitfw\P_8455\updater.bin

copy %AXF_DIR%\%AXF_NAME% c:\aitfw\P_8455\
pause

