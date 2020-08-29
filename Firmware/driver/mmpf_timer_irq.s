            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     irq.mac

            EXPORT  tc1_isr_a
            EXPORT  tc2_isr_a
            IMPORT  MMPF_TIMER1_ISR
            IMPORT  MMPF_TIMER2_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
tc1_isr_a
tc1            OS_IRQ_ENTRY

            ldr         r0, = MMPF_TIMER1_ISR
            mov         r14, pc
            bx          r0

tc1         OS_IRQ_EXIT


tc2_isr_a
tc2            OS_IRQ_ENTRY

            ldr         r0, = MMPF_TIMER2_ISR
            mov         r14, pc
            bx          r0

tc2         OS_IRQ_EXIT
            END
