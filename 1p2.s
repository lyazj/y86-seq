	irmovq	$1, %rax
	rrmovq	%rax, %rdx
	irmovq	$2, %rax
	addq	%rdx, %rax
	halt
	endtext
