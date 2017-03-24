	.file	"overflowattack.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"How many numbers to sum?"
.LC1:
	.string	"%d"
.LC2:
	.string	"%ld"
	.text
	.globl	sum_vals
	.type	sum_vals, @function
sum_vals:
.LFB39:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	subq	$48, %rsp
	.cfi_def_cfa_offset 64
	movl	$.LC0, %edi
	call	puts
	leaq	12(%rsp), %rsi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	__isoc99_scanf
	movl	$0, %ebx
	jmp	.L2
.L3:
	movslq	%ebx, %rax
	leaq	16(%rsp,%rax,8), %rsi
	movl	$.LC2, %edi
	movl	$0, %eax
	call	__isoc99_scanf
	addl	$1, %ebx
.L2:
	movl	12(%rsp), %edx
	cmpl	%edx, %ebx
	jl	.L3
	movl	$0, %eax
	movl	$0, %ecx
	jmp	.L4
.L5:
	movslq	%ecx, %rsi
	addl	16(%rsp,%rsi,8), %eax
	addl	$1, %ecx
.L4:
	cmpl	%edx, %ecx
	jl	.L5
	addq	$48, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE39:
	.size	sum_vals, .-sum_vals
	.section	.rodata.str1.1
.LC3:
	.string	"How did you get here?!?!"
	.text
	.globl	nevercalled
	.type	nevercalled, @function
nevercalled:
.LFB40:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$.LC3, %edi
	call	puts
	movl	$1, %edi
	call	exit
	.cfi_endproc
.LFE40:
	.size	nevercalled, .-nevercalled
	.section	.rodata.str1.1
.LC4:
	.string	"Sum of values: %d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB41:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$0, %eax
	call	sum_vals
	movl	%eax, %edx
	movl	$.LC4, %esi
	movl	$1, %edi
	movl	$0, %eax
	call	__printf_chk
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE41:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
