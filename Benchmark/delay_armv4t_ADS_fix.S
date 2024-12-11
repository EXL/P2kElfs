; The delay_bmips function for proper Motorola L7e-like benchmarking by @fkcoder/Siesta.

	AREA |asm.delay_bmips_fix|, CODE, READONLY
	CODE16
	EXPORT delay_bmips_fix

delay_bmips_fix
	push     {r1-r2}
	ldr      r2, =0x3FC0000  ; just an unused instruction for ignore watchdog's (?) reboot...
loop
	ldrb     r1, [r2]        ; same as ldr
	subs     r0, r0, #1
	bge      loop
	pop      {r1-r2}
	bx       lr

	END
