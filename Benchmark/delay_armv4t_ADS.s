; Thanks to @usernameak for porting it to ADS (Unified Assembler Language) ARM Assembler.

	AREA |asm.delay_bmips|, CODE, READONLY
	CODE32
	EXPORT delay_bmips

delay_bmips
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcc      delay_bmips_return
	subs     r0, r0, #1
	bcs      delay_bmips
delay_bmips_return
	bx       lr

	END
