// palinfinder.s, provided with Lab1 in TDT4258 autumn 2025
.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	// Here your execution starts
	bl check_palindrom
	cmp r0, #0
	beq _start_is_no_palindrom
	bl is_palindrom
	b _exit
	
	_start_is_no_palindrom:
	bl is_no_palindrom
	b _exit

	
check_palindrom:
	// Here you could check whether input is a palindrom or not
	ldr r1, =input
	mov r0, r1
	get_end_loop_start:
		add r1, #1
		ldrb r3, [r1]
		cmp r3, #0
		bne get_end_loop_start
	sub r1, #1
	cmp r0, r1
	beq not_palindrome // String under 2 characters
	add r1, #1
	sub r0, #1
	compare_char_loop_start:
	sub: // sub end pointer and skip all spaces
	    sub r1, #1
		ldrb r3, [r1]
		cmp r3, #' '
		beq sub
	add: // add start pointer and skip all spaces
		add r0, #1
		ldrb r2, [r0]
		cmp r2, #' '
		beq add

		cmp r3, #'?'
		beq wildcard
		cmp r3, #'#'
		beq wildcard
		cmp r2, #'?'
		beq wildcard
		cmp r2, #'#'
		beq wildcard
		b wildcard_end
	wildcard:
		mov r2, r3
		b upper_case1
	wildcard_end:
		cmp r3, #'a'
		ble upper_case0
		sub r3, #32
	upper_case0:
		cmp r2, #'a'
		ble upper_case1
		sub r2, #32
	upper_case1:
		cmp r3, r2
		bne not_palindrome
		cmp r0, r1
		blt compare_char_loop_start
	mov r0, #1
	bx lr
	not_palindrome:
	mov r0, #0
	bx lr
	
		
	
	
is_palindrom:
	// Switch on only the 5 rightmost LEDs
	// Write 'Palindrom detected' to UART
	mov r0, #0x1f
	ldr r1, =#0xff200000
	str r0, [r1]
	
	ldr r0, =palindrome_str
	ldr r1, =#0xff201000
	palindrome_uart_loop:
	ldrb r2, [r0]
	strb r2, [r1]
	add r0, #1
	cmp r2, #0
	bne palindrome_uart_loop
	bx lr
	
	
is_no_palindrom:
	// Switch on only the 5 leftmost LEDs
	// Write 'Not a palindrom' to UART
	mov r0, #0xfe0
	ldr r1, =#0xff200000
	str r0, [r1]
	
	ldr r0, =not_palindrome_str
	ldr r1, =#0xff201000
	no_palindrome_uart_loop:
	ldrb r2, [r0]
	strb r2, [r1]
	add r0, #1
	cmp r2, #0
	bne no_palindrome_uart_loop
	bx lr
	
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "Grav ned den varg"
	not_palindrome_str: .asciz "Not a palindrom"
	palindrome_str: .asciz "Palindrom detected"
.end
