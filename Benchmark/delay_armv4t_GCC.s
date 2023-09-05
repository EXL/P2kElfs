/* See delay_armv4t_ADS.s */

.global delay_bmips

delay_bmips:
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
delay_bmips_return:
	bx       lr
