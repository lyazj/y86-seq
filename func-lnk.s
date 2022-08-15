	irmovq	$0x200, %rsp
	irmovq	$0, %rdi
	call	func
	irmovq	$8, %rdi
	call	func
	halt
	.align 16
func:
	pushq	%rbp
	rrmovq	%rsp, %rbp
	irmovq	$23, %rax
	addq	%rax, %rdi
	irmovq	$-16, %rax
	andq	%rax, %rdi
	subq	%rdi, %rsp
	mrmovq	-8(%rbp), %rax
	irmovq	$inc, %rdx
	mrmovq	0(%rdx), %rdx
	addq	%rdx, %rax
	rmmovq	%rax, -8(%rbp)
	rrmovq	%rbp, %rsp
	popq	%rbp
	ret
	endtext
	.align 8
inc:
	.quad	1
	.pos	0x200
stack:
	enddata
