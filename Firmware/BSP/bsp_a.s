           PRESERVE8 ; For RealView Assembler

;********************************************************************************************************
;                                                uC/OS-II
;                                          The Real-Time Kernel
;
;                               (c) Copyright 1992-2004, Micrium, Weston, FL
;                                          All Rights Reserved
;
;                                            ATMEL AT91RM9200
;                                         Board Support Package
;
;
; File         : BSP_A.S
; Originally by: Jean-Denis Hatier
;
;********************************************************************************************************

            IMPORT  pIRQStkTop
            IMPORT  pFIQStkTop

            IMPORT  OS_CPU_IRQ_ISR

            EXPORT  INT_Initialize
            EXPORT  SetStackPointers

NO_INT      EQU     0xC0                         ; Mask used to disable interrupts (Both FIR and IRQ)
SVC32_MODE  EQU     0x13
FIQ32_MODE  EQU     0x11
IRQ32_MODE  EQU     0x12



            AREA    BSP_A, CODE, READONLY
            CODE32

INT_Vectors
INT_Reset_Vector
        B       INT_Reset_Vector                ; not support yet
INT_Undef_Vector
        B       INT_Undef_Vector                ; not support yet
INT_Software_Vector
        B       INT_Software_Vector             ; not support yet
INT_Prefetch_Vector
        B       INT_Prefetch_Vector             ; not support yet
INT_Abort_Vector
        B       INT_Abort_Vector                ; not support yet
INT_Reserved_Vector
        B       INT_Reserved_Vector
INT_IRQ_Vector
        LDR     PC, INT_IRQ_Addr
INT_FIG_Vector
        B       INT_FIG_Vector                  ; not support yet

INT_Reset_Addr
        DCD     0
INT_Undef_Addr
        DCD     0
INT_Software_Addr
        DCD     0
INT_Prefetch_Addr
        DCD     0
INT_Abort_Addr
        DCD     0
INT_Reserved_Addr
        DCD     0
INT_IRQ_Addr
        DCD     OS_CPU_IRQ_ISR
INT_FIQ_Addr
        DCD     0



INT_Initialize

        STMDB       SP!, {R0-R9,LR}             ; save working registers

        ; AIC Initialization

        ; Copy vector table to address 0x00000000
        MOV     R8, #0x00                       ; Pickup address of vector table (0x00000000)
        LDR     R9, =INT_Vectors                ; Pickup address of our vector table
        LDMIA   R9!,{R0-R7}                     ; Load vector table values into registers
        STMIA   R8!,{R0-R7}                     ; Store vector table values at correct address
        LDMIA   R9!,{R0-R7}                     ; Load vector table values into registers
        STMIA   R8!,{R0-R7}                     ; Store vector table values at correct address

        LDMIA   SP!,{R0-R9,LR}                  ; restore working registers

        BX      LR                              ; return


SetStackPointers
        MRS     R0, CPSR                        ; Save CPSR

        MSR     CPSR_c, #(NO_INT | FIQ32_MODE)  ; Set FIQ stack
        LDR     R1, =pFIQStkTop
        LDR     SP, [R1]

        MSR     CPSR_c, #(NO_INT | IRQ32_MODE)  ; Set IRQ stack
        LDR     R1, =pIRQStkTop
        LDR     SP, [R1]

        MSR     CPSR_cxsf, R0                   ; Restore CPSR back to original value
        BX      LR                              ; Return


    END