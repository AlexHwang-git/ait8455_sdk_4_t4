            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------

			INCLUDE irq.mac

            EXPORT  pwm_isr_a
            EXPORT  ibc_isr_a
            EXPORT  gpio_isr_a
            EXPORT  display_isr_a
            ;EXPORT  sd_isr_a
            
            IMPORT  MMPF_PWM_ISR
            ;IMPORT  MMPF_Display_GpioISR
            IMPORT  MMPF_PIO_ISR
            IMPORT  MMPF_DISPLAY_ISR
            ;IMPORT  MMPF_SD_ISR
            IMPORT  MMPF_Display_IbcISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
            AREA        AIT_IRQ, CODE, READONLY

;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------

;hif_isr_a
;host        OS_IRQ_ENTRY
;
;            LDR         R0, = MMPF_HIF_HifISR
;            MOV         LR, PC
;            BX          R0

host        OS_IRQ_EXIT


ibc_isr_a
ibc         OS_IRQ_ENTRY
            ldr         r0, = MMPF_Display_IbcISR
            mov         r14, pc
            bx          r0

ibc         OS_IRQ_EXIT

gpio_isr_a
gpio        OS_IRQ_ENTRY

            ldr         r0, = MMPF_PIO_ISR
;            ldr         r0, = MMPF_Display_GpioISR
            mov         r14, pc
            bx          r0

gpio        OS_IRQ_EXIT




display_isr_a
display            OS_IRQ_ENTRY

            ldr         r0, =MMPF_DISPLAY_ISR
            mov         r14, pc
            bx          r0

display         OS_IRQ_EXIT

;sd_isr_a
;sd            OS_IRQ_ENTRY
;            ldr         r0, =MMPF_SD_ISR
;            mov         r14, pc
;            bx          r0
;sd         OS_IRQ_EXIT



;spi_isr_a
;spi            OS_IRQ_ENTRY
;            ldr         r0, =MMPF_SPI_ISR
;            mov         r14, pc
;            bx          r0
;spi         OS_IRQ_EXIT
;pcm0_isr_a
;pcm0            OS_IRQ_ENTRY
;
;            ldr         r0, =RTNA_PCM0_ISR
;            mov         r14, pc
;            bx          r0

;pcm0         OS_IRQ_EXIT
;
;pcm1_isr_a
;pcm1            OS_IRQ_ENTRY
;
;            ldr         r0, =RTNA_PCM1_ISR
;            mov         r14, pc
;            bx          r0
;
;pcm1         OS_IRQ_EXIT

pwm_isr_a
pwm            OS_IRQ_ENTRY

            ldr         r0, =MMPF_PWM_ISR
            mov         r14, pc
            bx          r0

pwm         OS_IRQ_EXIT

            END
