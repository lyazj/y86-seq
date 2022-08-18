# vim:ft=asm
	.pos	0x100
	.align 8
src:
	.quad	0x00a
	.quad	0x0b0
	.quad	0xc00
dest:
	.quad	0x111
	.quad	0x222
	.quad	0x333
	enddata
	.pos	0x200
stack:
	.pos	0
start:
	irmovq	$stack, %rsp
	xorq	%rbp, %rbp
	irmovq	$3, %rdx
	irmovq	$dest, %rsi
	irmovq	$src, %rdi
	call	copy_block
	halt
# long copy_block(long *src, long *dst, long len);
# 	src in %rdi, dst in %rsi, len in %rdx
# 	1 in %rbx, 8 in %rbp, val in %rcx
copy_block:
	xorq	%rax, %rax
	andq	%rdx, %rdx
	jle	L1
	pushq	%rbx
	pushq	%rbp
	irmovq	$1, %rbx
	irmovq	$8, %rbp
L0:
	mrmovq	0(%rdi), %rcx
	addq	%rbp, %rdi
	rmmovq	%rcx, 0(%rsi)
	addq	%rbp, %rsi
	xorq	%rcx, %rax
	subq	%rbx, %rdx
	jg	L0
	popq	%rbp
	popq	%rbx
L1:
	ret
	endtext
