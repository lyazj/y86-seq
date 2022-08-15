	irmovq	$3, %rdi
	rrmovq	%rdi, %rax
	xorq	%rdx, %rdx
	subq	%rax, %rdx
	cmovg 	%rdx, %rax
	halt
	endtext
