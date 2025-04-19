.global _start
_start:

	.equ LEDs,  0xFF200000
	.equ TIMER, 0xFF202000
    .equ KEY_BASE, 0xFF200050 
	.equ DELAY, 12500000

	#Set up the stack pointer
    la sp, 0x20000 

    csrw mstatus, zero # write 0 to the mstatus register to make sure we dont get processor interupts 

    #setup keys 
    la t1, KEY_BASE 
    li t2, 0b1111
    sw t2, 8(t1) # have it request interupt from all keys 
    sw t2, 12(t1) # clear edge capture as precaution 


    #setup timer
    la t0, TIMER
    li t1, DELAY # counter low order 16 bits 
	sw zero, (t0)
	sw t1, 0x8(t0) # low 16 bits of the counter value
    srli t3, t1, 16 # shift to get upper 16 bits
	sw t3, 0xc(t0) # store upper 16 bits into count start the counter
    li t1, 0b0111 
    sw t1, 4(t0) # Set start, continous mode, and ito all to 1 (ito is )


    li t0, 0x40000 # turn on processor interupts from the KEYS  (line 18 which is bit 18 of 32 bit word)
    csrs mie, t0 # sets the mie bit to 1 as well 

    li t0, 0x10000 # bit 16 is ON
    csrs mie, t0 # sets the mie bit to 1 as wwell for irq 16 

    la t0, interruptHandler 
    csrw mtvec, t0 #set the address inside mtvec to be the address of the Interupt handler 


    li t0, 8 
    csrs mstatus, t0 
    

	
	LOOP:
        la t0, LEDs
        la t1, COUNT
        lw t2, (t1) # load curr count from t2 and store/write it to the leds
        sw t2, (t0)
	j LOOP

interruptHandler:
	#Code not shown

    addi sp, sp, -16
    sw t0, 0(sp)
    sw t1, 4(sp)
    sw t2, 8(sp)
    sw ra, 0xC(sp)

    li t0, 0x7FFFFFFF
    csrr t1, mcause # read mcause into t1 
    and t1,t1, t0 # and it with t0 to get rid of the 31st bit cus it doesnt contain teh address 

     
    #key intrupt 
    checkKeys: 
    li t0, 18 
    beq t0, t1, handleKeys 

    
    checkTimer: 
    #timer intrupt
    li t0, 16 
    beq t0, t1, handleTimer      



    handleKeys: 
    addi sp,sp,-12
    sw t0, 0(sp)
    sw t1, 4(sp)
    sw ra, 8(sp)
   
    la s0, KEY_BASE
    lw s2, RUN 
    xori s2,s2,1 #flip all the bits for run (1 bit)
    sw s2, (s0) 
    li s2, 0b1111 
    sw s2, 12(s0) #clear edge capture for keys


    #! idk if acc needed so... <- check here if error (even at beginning)
    lw t0, 0(sp)
    lw t1, 4(sp)
    lw ra, 8(sp)
    addi sp,sp, 12 
    j endInterupt


    handleTimer:    
    addi sp,sp, -12
    sw t0, 0(sp)
    sw t1, 4(sp)
    sw ra, 8(sp)

    la s0, COUNT 
    la s1, RUN  
    lw t0, (s1) 
    bnez t0, incCount # increment iff run is still 1 at timeout


    lw t0, 0(sp)
    lw t1, 4(sp)
    lw ra, 8(sp)
    addi sp,sp, 12
    j endInterupt

    incCount: 

    lw s2, (s0) # get the count at the time that it has been intrupted/timed out 
    li t0, 255 
    bgt s2, t0, resetZero 
    addi s2, s2,1  
    sw s2, (s0)

    lw t0, 0(sp)
    lw t1, 4(sp)
    lw ra, 8(sp)
    addi sp,sp, 12
    j endInterupt

    resetZero: 
    #! might have to load a 0 and then store 
    sw zero, (s0)
    j endInterupt
endInterupt: 
    lw t0, 0(sp)
    lw t1, 4(sp)
    lw t2, 8(sp)
    lw ra, 0xC(sp)
    addi sp, sp, 16 
mret # retores mie bit back to 1 enabling interupts because we are leaving the interupt handler 


.data
/* Global variables */
.global  COUNT
COUNT:  .word    0x0            # used by timer

.global  RUN                    # used by pushbutton KEYs
RUN:    .word    0x1            # initial value to increment COUNT

.end
