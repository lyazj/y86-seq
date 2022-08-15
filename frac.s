	.pos	0x100
_stack:
	.pos	0
_start:
	irmovq	$_stack, %rsp
	xorq	%rbp, %rbp
	call	main
	halt

# long frac(long n);
# 	n in %rdi
frac:
	irmovq	$1, %rax
	andq	%rdi, %rdi
	je	.L0
	pushq	%rbx
	rrmovq	%rdi, %rbx
	subq	%rax, %rdi
	call	frac
	imulq	%rbx, %rax
	popq	%rbx
.L0:
	ret

# int main(void);
main:
	irmovq	$8, %rdi
	jmp	frac
	endtext
