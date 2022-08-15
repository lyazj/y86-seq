	rrmovq	%rdi, %rbx
	addq	%rbx, %rax
	irmovq	$0x4, %rdx
	subq	%rdx, %rax
	xorq	%rdx, %rdx
	mrmovq	0x8(%rdx), %rdi
	rmmovq	%rdi, 0x8(%rdx)
	irmovq	$0x8, %rsp
	pushq	%rbx
	popq	%rbx
	endtext
