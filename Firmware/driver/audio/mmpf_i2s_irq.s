            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     ..\irq.mac

            EXPORT  i2s_isr_a
            IMPORT  MMPF_AFE_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
i2s_isr_a
i2s            OS_IRQ_ENTRY

            ldr         r0, = MMPF_AFE_ISR
            mov         r14, pc
            bx          r0

i2s         OS_IRQ_EXIT
            END
