           PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     ..\irq.mac

            EXPORT  afe_isr_a
            IMPORT  MMPF_AFE_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
afe_isr_a
afe            OS_IRQ_ENTRY

            ldr         r0, = MMPF_AFE_ISR
            mov         r14, pc
            bx          r0

afe         OS_IRQ_EXIT
            END
