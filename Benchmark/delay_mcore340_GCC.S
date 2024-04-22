/* Thanks to the ChatGPT-4 for helping me port it from GCC ARM to GCC M*CORE. */

.global delay_bmips
.type delay_bmips, @function

delay_bmips:
	mov r0, r0
delay_bmips_mcore_loop:
	declt r2
	bf delay_bmips_mcore_loop
	jmp r15
