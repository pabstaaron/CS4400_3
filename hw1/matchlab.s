	.file	"matchlab.c"
	.text
	.p2align 4,,15
	.globl	isDig
	.type	isDig, @function
isDig:
.LFB39:
	.cfi_startproc
	subl	$48, %edi
	xorl	%eax, %eax
	cmpb	$9, %dil
	setbe	%al
	ret
	.cfi_endproc
.LFE39:
	.size	isDig, .-isDig
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	""
	.text
	.p2align 4,,15
	.globl	aMode
	.type	aMode, @function
aMode:
.LFB40:
	.cfi_startproc
	movzbl	(%rdi), %eax
	testb	%al, %al
	je	.L15
	leaq	1(%rdi), %rdx
	xorl	%ecx, %ecx
	xorl	%esi, %esi
	jmp	.L11
	.p2align 4,,10
	.p2align 3
.L21:
	cmpb	$109, %al
	je	.L9
	andl	$1, %esi
	je	.L15
	cmpb	$118, %al
	.p2align 4,,4
	jne	.L15
	movl	$1, %ecx
	movl	$1, %esi
.L6:
	addq	$1, %rdx
	movzbl	-1(%rdx), %eax
	testb	%al, %al
	je	.L20
.L11:
	testl	%ecx, %ecx
	je	.L21
	cmpl	$1, %ecx
	jne	.L7
	cmpb	$118, %al
	.p2align 4,,4
	je	.L9
	cmpl	$1, %esi
	.p2align 4,,3
	jle	.L15
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L15
	addq	$1, %rdx
	movzbl	-1(%rdx), %eax
	movl	$2, %ecx
	movl	$1, %esi
	testb	%al, %al
	jne	.L11
	.p2align 4,,10
	.p2align 3
.L20:
	subl	$1, %esi
	movl	$.LC0, %eax
	cmpl	$3, %esi
	cmovb	%rdi, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L7:
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L15
	.p2align 4,,10
	.p2align 3
.L9:
	addl	$1, %esi
	jmp	.L6
	.p2align 4,,10
	.p2align 3
.L15:
	movl	$.LC0, %eax
	ret
	.cfi_endproc
.LFE40:
	.size	aMode, .-aMode
	.p2align 4,,15
	.globl	isCap
	.type	isCap, @function
isCap:
.LFB41:
	.cfi_startproc
	subl	$65, %edi
	xorl	%eax, %eax
	cmpb	$25, %dil
	setbe	%al
	ret
	.cfi_endproc
.LFE41:
	.size	isCap, .-isCap
	.p2align 4,,15
	.globl	bMode
	.type	bMode, @function
bMode:
.LFB42:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movzbl	(%rdi), %ebx
	movl	$8, %edi
	call	malloc
	testb	%bl, %bl
	je	.L24
	leaq	1(%rbp), %rcx
	movl	%ebx, %edx
	movl	$2, %r9d
	movl	$1, %edi
	xorl	%r8d, %r8d
	xorl	%esi, %esi
	jmp	.L39
	.p2align 4,,10
	.p2align 3
.L48:
	cmpb	$103, %dl
	je	.L37
	cmpl	$3, %r8d
	jle	.L28
	leal	-48(%rdx), %esi
	cmpb	$9, %sil
	ja	.L28
	movb	%dl, (%rax)
	movl	$1, %r8d
	movl	$1, %esi
.L27:
	addq	$1, %rcx
	movzbl	-1(%rcx), %edx
	testb	%dl, %dl
	je	.L47
.L39:
	testw	%si, %si
	je	.L48
	cmpw	$1, %si
	je	.L49
	cmpw	$2, %si
	je	.L50
	cmpw	$3, %si
	je	.L51
	cmpw	$4, %si
	jne	.L27
	subl	$65, %edx
	cmpb	$25, %dl
	jbe	.L37
	.p2align 4,,10
	.p2align 3
.L28:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	movl	$.LC0, %eax
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L37:
	.cfi_restore_state
	addq	$1, %rcx
	movzbl	-1(%rcx), %edx
	addl	$1, %r8d
	testb	%dl, %dl
	jne	.L39
	.p2align 4,,10
	.p2align 3
.L47:
	movl	$2, %edi
	call	malloc
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
.L42:
	movl	%edx, %ecx
	movb	%bl, (%rax,%rdx,2)
	andl	$7, %ecx
	addl	$48, %ecx
	movb	%cl, 1(%rax,%rdx,2)
	addq	$1, %rdx
	movzbl	0(%rbp,%rdx), %ebx
	testb	%bl, %bl
	jne	.L42
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L49:
	.cfi_restore_state
	leal	-48(%rdx), %r10d
	cmpb	$9, %r10b
	ja	.L52
	movswq	%di, %r10
	addl	$1, %edi
	addl	$1, %r8d
	cmpw	$5, %di
	movb	%dl, (%rax,%r10)
	jle	.L27
	jmp	.L28
	.p2align 4,,10
	.p2align 3
.L50:
	cmpb	$110, %dl
	je	.L37
	subl	$2, %r8d
	cmpl	$3, %r8d
	ja	.L28
	cmpb	%dl, (%rax)
	jne	.L28
	movl	$1, %r8d
	movl	$3, %esi
	jmp	.L27
	.p2align 4,,10
	.p2align 3
.L51:
	movslq	%r9d, %r10
	cmpb	%dl, (%rax,%r10)
	jne	.L36
	addl	$2, %r9d
	jmp	.L27
	.p2align 4,,10
	.p2align 3
.L52:
	cmpb	$110, %dl
	jne	.L28
	cmpl	$3, %r8d
	jg	.L28
	movl	$1, %r8d
	movl	$2, %esi
	jmp	.L27
.L36:
	subl	$65, %edx
	cmpb	$25, %dl
	ja	.L28
	movl	$4, %esi
	jmp	.L27
.L24:
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	movl	$2, %edi
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	jmp	malloc
	.cfi_endproc
.LFE42:
	.size	bMode, .-bMode
	.p2align 4,,15
	.globl	cMode
	.type	cMode, @function
cMode:
.LFB43:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movzbl	(%rdi), %ebx
	movl	$64, %edi
	call	malloc
	testb	%bl, %bl
	je	.L54
	addq	$1, %rbp
	movl	%ebx, %edx
	movl	$1, %r9d
	movq	%rbp, %rcx
	movl	$1, %r10d
	xorl	%esi, %esi
	xorl	%r8d, %r8d
	jmp	.L70
	.p2align 4,,10
	.p2align 3
.L91:
	cmpb	$98, %dl
	je	.L69
	testb	$1, %r8b
	je	.L83
	leal	-65(%rdx), %esi
	cmpb	$25, %sil
	ja	.L83
	movb	%dl, (%rax)
	movl	$1, %esi
	movl	$1, %r8d
	.p2align 4,,10
	.p2align 3
.L57:
	addq	$1, %rcx
	movzbl	-1(%rcx), %edx
	testb	%dl, %dl
	je	.L58
.L70:
	testw	%si, %si
	je	.L91
	cmpw	$1, %si
	je	.L92
	cmpw	$2, %si
	je	.L93
	cmpw	$3, %si
	je	.L94
	cmpw	$4, %si
	jne	.L57
	subl	$48, %edx
	cmpb	$9, %dl
	ja	.L95
	.p2align 4,,10
	.p2align 3
.L69:
	addl	$1, %r8d
	jmp	.L57
	.p2align 4,,10
	.p2align 3
.L94:
	movslq	%r9d, %rdi
	cmpb	%dl, -1(%rax,%rdi)
	je	.L96
	testl	%r9d, %r9d
	jns	.L97
	.p2align 4,,10
	.p2align 3
.L83:
	xorl	%r10d, %r10d
.L58:
	cmpl	$3, %r8d
	jle	.L98
.L71:
	movl	$.LC0, %eax
.L88:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L92:
	.cfi_restore_state
	leal	-65(%rdx), %r11d
	cmpb	$25, %r11b
	ja	.L99
	movslq	%r9d, %rdi
	addl	$1, %r8d
	addl	$1, %r9d
	movb	%dl, (%rax,%rdi)
	jmp	.L57
	.p2align 4,,10
	.p2align 3
.L93:
	cmpb	$120, %dl
	je	.L69
	cmpl	$2, %r8d
	jg	.L83
	movslq	%r9d, %rsi
	cmpb	%dl, -1(%rax,%rsi)
	jne	.L71
	subl	$1, %r9d
	movl	$3, %esi
	movl	$1, %r8d
	jmp	.L57
	.p2align 4,,10
	.p2align 3
.L99:
	testb	$1, %r8b
	je	.L83
	cmpb	$120, %dl
	jne	.L83
	movl	$2, %esi
	movl	$1, %r8d
	jmp	.L57
	.p2align 4,,10
	.p2align 3
.L97:
	subl	$48, %edx
	cmpb	$9, %dl
	ja	.L83
	movl	$4, %esi
	jmp	.L57
	.p2align 4,,10
	.p2align 3
.L95:
	xorl	%r10d, %r10d
	jmp	.L57
	.p2align 4,,10
	.p2align 3
.L98:
	cmpw	$1, %r10w
	jne	.L71
	movl	$128, %edi
	call	malloc
	movq	%rbp, %rdi
	xorl	%edx, %edx
	jmp	.L75
	.p2align 4,,10
	.p2align 3
.L73:
	movb	%bl, (%rax,%rcx)
	addl	$1, %edx
.L74:
	addq	$1, %rdi
	movzbl	-1(%rdi), %ebx
	testb	%bl, %bl
	je	.L88
.L75:
	cmpb	$71, %bl
	movslq	%edx, %rcx
	jne	.L73
	movb	$71, (%rax,%rcx)
	movb	$71, 1(%rax,%rcx)
	addl	$3, %edx
	movb	$71, 2(%rax,%rcx)
	jmp	.L74
	.p2align 4,,10
	.p2align 3
.L96:
	subl	$1, %r9d
	jmp	.L57
.L54:
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	movl	$128, %edi
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	jmp	malloc
	.cfi_endproc
.LFE43:
	.size	cMode, .-cMode
	.section	.rodata.str1.1
.LC1:
	.string	"no"
.LC2:
	.string	"yes"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB44:
	.cfi_startproc
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	movq	8(%rsi), %rax
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	cmpb	$45, (%rax)
	je	.L116
	movl	$1, %eax
	xorl	%r13d, %r13d
	movl	$97, %ebp
.L101:
	cmpl	%edi, %eax
	jge	.L110
	subl	$1, %edi
	movslq	%eax, %rdx
	subl	%eax, %edi
	leaq	(%rsi,%rdx,8), %rbx
	addq	%rdi, %rdx
	leaq	8(%rsi,%rdx,8), %r14
	jmp	.L111
	.p2align 4,,10
	.p2align 3
.L121:
	testw	%r13w, %r13w
	je	.L117
.L108:
	addq	$8, %rbx
	cmpq	%r14, %rbx
	je	.L110
.L111:
	cmpb	$97, %bpl
	je	.L118
	cmpb	$98, %bpl
	je	.L119
	cmpb	$99, %bpl
	je	.L120
.L105:
	cmpb	$0, (%r12)
	je	.L121
	testw	%r13w, %r13w
	jne	.L109
	movl	$.LC2, %edi
	addq	$8, %rbx
	call	puts
	cmpq	%r14, %rbx
	jne	.L111
.L110:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	xorl	%eax, %eax
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L118:
	.cfi_restore_state
	movq	(%rbx), %rdi
	call	aMode
	movq	%rax, %r12
	jmp	.L105
	.p2align 4,,10
	.p2align 3
.L109:
	movq	%r12, %rdi
	call	puts
	.p2align 4,,2
	jmp	.L108
	.p2align 4,,10
	.p2align 3
.L117:
	movl	$.LC1, %edi
	call	puts
	.p2align 4,,3
	jmp	.L108
	.p2align 4,,10
	.p2align 3
.L119:
	movq	(%rbx), %rdi
	call	bMode
	movq	%rax, %r12
	.p2align 4,,2
	jmp	.L105
	.p2align 4,,10
	.p2align 3
.L120:
	movq	(%rbx), %rdi
	call	cMode
	movq	%rax, %r12
	jmp	.L105
.L116:
	movzbl	1(%rax), %ebp
	cmpb	$116, %bpl
	je	.L122
	movq	16(%rsi), %rdx
	xorl	%eax, %eax
	cmpb	$116, 1(%rdx)
	sete	%al
	xorl	%r13d, %r13d
	addl	$2, %eax
	cmpb	$116, 1(%rdx)
	sete	%r13b
	jmp	.L101
.L122:
	movq	16(%rsi), %rax
	movl	$1, %r13d
	movzbl	1(%rax), %ebp
	movl	$3, %eax
	jmp	.L101
	.cfi_endproc
.LFE44:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
