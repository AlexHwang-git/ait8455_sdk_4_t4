;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     ..\irq.mac

            EXPORT  pcm_isr_a
            IMPORT  MMPF_PCM_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
pcm_isr_a
pcm            OS_IRQ_ENTRY

            ldr         r0, = MMPF_PCM_ISR
            mov         r14, pc
            bx          r0

pcm         OS_IRQ_EXIT
            END
