	.pos	0x100
_stack:
	.pos	0
_start:
	irmovq	$_stack, %rsp
	xorq	%rbp, %rbp
	call	main
	halt
main:
	irmovq	$retp, %rax
	pushq	%rax
	xorq	%rax, %rax
	jne	.L0
	irmovq	$1, %rax	# expected routine
	halt
.L0:
	ret			# mispredicted routine
	irmovq	$2, %rax	# unexpected routine
	halt
retp:
	irmovq	$3, %rax	# mispredicted routine (subsequence)
	halt
	endtext
