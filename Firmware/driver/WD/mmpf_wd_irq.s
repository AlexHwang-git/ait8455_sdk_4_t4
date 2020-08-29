            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     ..\irq.mac

            EXPORT  dma_wd_a
            IMPORT  MMPF_WD_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
dma_wd_a
dma        OS_IRQ_ENTRY

            ldr         r0, = MMPF_WD_ISR
            mov         r14, pc
            bx          r0

dma        OS_IRQ_EXIT
            END
