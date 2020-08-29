            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

            INCLUDE     ..\irq.mac

            EXPORT  usb_isr_a
            IMPORT  MMPF_USB_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit

;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
usb_isr_a
usb         OS_IRQ_ENTRY

            ldr         r0, = MMPF_USB_ISR
            mov         r14, pc
            bx          r0

usb         OS_IRQ_EXIT

            END
