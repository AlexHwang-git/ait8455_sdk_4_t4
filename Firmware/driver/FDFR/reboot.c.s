; generated by ARM C Compiler, ADS1.2 [Build 848]

; commandline [-O2 -browseinfo "0xff" -S   -g+ -apcs /interwork -cpu ARM926EJ-S -fk -D_LOADER_ -J:cw:]
        CODE32

        AREA ||.text||, CODE, READONLY

reboot PROC
		ldr		r1, =0x0		;; Firmware starting address, refer target catter file
		mov		pc, r1
        ENDP



        EXPORT reboot

        IMPORT ||Lib$$Request$$armlib||, WEAK

        KEEP ||BuildAttributes$$ARM_ISAv5$M$E$P$PE$A:L22$X:L11$S22$IW$USESV6$~STKCKD$USESV7$~SHL$OSPACE$PRES8||
||BuildAttributes$$ARM_ISAv5$M$E$P$PE$A:L22$X:L11$S22$IW$USESV6$~STKCKD$USESV7$~SHL$OSPACE$PRES8|| EQU 0

        ASSERT {ENDIAN} = "little"
        ASSERT {SWST} = {FALSE}
        ASSERT {NOSWST} = {TRUE}
        ASSERT {ROPI} = {FALSE}
        ASSERT {RWPI} = {FALSE}
        ASSERT {NOT_SHL} = {TRUE}
        ASSERT {FULL_IEEE} = {FALSE}
        ASSERT {SHL1} = {FALSE}
        ASSERT {SHL2} = {FALSE}
        END
