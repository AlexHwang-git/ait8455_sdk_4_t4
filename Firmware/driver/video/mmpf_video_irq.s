            PRESERVE8 ; For RealView Assembler
;--------------------------------------------
;- Export assembly symbols for C function to call
;- Import C function symbols for assembly to call
;--------------------------------------------
            AREA        AIT_IRQ, CODE, READONLY

			INCLUDE     ..\irq.mac

            ;EXPORT  video_isr_a
            ;EXPORT  memc_isr_a
            EXPORT  h264enc_isr_a
            ;IMPORT  MMPF_VIDEO_ISR
            ;IMPORT  MMPF_MEMC_ISR
            IMPORT  MMPF_H264ENC_ISR

            IMPORT  OSTCBCur
            IMPORT  OSIntNesting
            IMPORT  OSIntExit
            
;--------------------------------------------
;- Add ISR handler for A-I-T here
;--------------------------------------------
;video_isr_a
;video         OS_IRQ_ENTRY
;
;            ldr         r0, = MMPF_VIDEO_ISR
;            mov         r14, pc
;            bx          r0
;
;video         OS_IRQ_EXIT

;memc_isr_a
;memc          OS_IRQ_ENTRY

;            ldr         r0, = MMPF_MEMC_ISR
;            mov         r14, pc
;            bx          r0

;memc          OS_IRQ_EXIT

h264enc_isr_a
h264enc       OS_IRQ_ENTRY

            ldr         r0, = MMPF_H264ENC_ISR
            mov         r14, pc
            bx          r0

h264enc       OS_IRQ_EXIT

            END
