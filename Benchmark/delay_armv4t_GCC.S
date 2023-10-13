/* Thanks to @usernameak for helping me port it to GCC ARM Assembler. */

.global delay_bmips

.thumb
delay_bmips:
	bx pc
.arm
	b delay_bmips_arm

delay_bmips_arm:
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
	bcs      delay_bmips_arm
delay_bmips_return:
	bx       lr
