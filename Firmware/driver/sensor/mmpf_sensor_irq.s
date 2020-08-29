            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     ..\irq.mac

            EXPORT  vif_isr_a
            EXPORT  isp_isr_a
            IMPORT  MMPF_VIF_ISR
            IMPORT  MMPF_ISP_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
vif_isr_a
vif         OS_IRQ_ENTRY

            ldr         r0, = MMPF_VIF_ISR
            mov         r14, pc
            bx          r0

vif         OS_IRQ_EXIT

isp_isr_a
isp         OS_IRQ_ENTRY

            ldr         r0, = MMPF_ISP_ISR
            mov         r14, pc
            bx          r0

isp         OS_IRQ_EXIT




            END
