	.pos	0x200
	.pos	0
	irmovq	$0x200, %rsp
	irmovq	$0, %rdi
	call	0x100
	irmovq	$8, %rdi
	call	0x100
	halt
	.pos	0x100
	pushq	%rbp
	rrmovq	%rsp, %rbp
	irmovq	$23, %rax
	addq	%rax, %rdi
	irmovq	$-16, %rax
	andq	%rax, %rdi
	subq	%rdi, %rsp
	mrmovq	-8(%rbp), %rax
	irmovq	$1, %rdx
	addq	%rdx, %rax
	rmmovq	%rax, -8(%rbp)
	rrmovq	%rbp, %rsp
	popq	%rbp
	ret
	endtext
