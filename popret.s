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
	pushq	%rsp
	popq	%rsp		# load to %rsp before ret
	ret
retp:
	halt			# normal routine: return 0
	endtext
