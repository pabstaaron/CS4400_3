
demo.so:     file format elf64-x86-64


Disassembly of section .init:

00000000000005b0 <_init>:
 5b0:	48 83 ec 08          	sub    $0x8,%rsp
 5b4:	48 8b 05 1d 0a 20 00 	mov    0x200a1d(%rip),%rax        # 200fd8 <_DYNAMIC+0x1a0>
 5bb:	48 85 c0             	test   %rax,%rax
 5be:	74 05                	je     5c5 <_init+0x15>
 5c0:	e8 1b 00 00 00       	callq  5e0 <_init+0x30>
 5c5:	48 83 c4 08          	add    $0x8,%rsp
 5c9:	c3                   	retq   

Disassembly of section .plt:

00000000000005d0 <.plt>:
 5d0:	ff 35 32 0a 20 00    	pushq  0x200a32(%rip)        # 201008 <_GLOBAL_OFFSET_TABLE_+0x8>
 5d6:	ff 25 34 0a 20 00    	jmpq   *0x200a34(%rip)        # 201010 <_GLOBAL_OFFSET_TABLE_+0x10>
 5dc:	0f 1f 40 00          	nopl   0x0(%rax)

Disassembly of section .plt.got:

00000000000005e0 <.plt.got>:
 5e0:	ff 25 f2 09 20 00    	jmpq   *0x2009f2(%rip)        # 200fd8 <_DYNAMIC+0x1a0>
 5e6:	66 90                	xchg   %ax,%ax
 5e8:	ff 25 0a 0a 20 00    	jmpq   *0x200a0a(%rip)        # 200ff8 <_DYNAMIC+0x1c0>
 5ee:	66 90                	xchg   %ax,%ax

Disassembly of section .text:

00000000000005f0 <deregister_tm_clones>:
 5f0:	48 8d 3d 29 0a 20 00 	lea    0x200a29(%rip),%rdi        # 201020 <_edata>
 5f7:	48 8d 05 29 0a 20 00 	lea    0x200a29(%rip),%rax        # 201027 <a+0x3>
 5fe:	55                   	push   %rbp
 5ff:	48 29 f8             	sub    %rdi,%rax
 602:	48 89 e5             	mov    %rsp,%rbp
 605:	48 83 f8 0e          	cmp    $0xe,%rax
 609:	76 15                	jbe    620 <deregister_tm_clones+0x30>
 60b:	48 8b 05 b6 09 20 00 	mov    0x2009b6(%rip),%rax        # 200fc8 <_DYNAMIC+0x190>
 612:	48 85 c0             	test   %rax,%rax
 615:	74 09                	je     620 <deregister_tm_clones+0x30>
 617:	5d                   	pop    %rbp
 618:	ff e0                	jmpq   *%rax
 61a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 620:	5d                   	pop    %rbp
 621:	c3                   	retq   
 622:	0f 1f 40 00          	nopl   0x0(%rax)
 626:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
 62d:	00 00 00 

0000000000000630 <register_tm_clones>:
 630:	48 8d 3d e9 09 20 00 	lea    0x2009e9(%rip),%rdi        # 201020 <_edata>
 637:	48 8d 35 e2 09 20 00 	lea    0x2009e2(%rip),%rsi        # 201020 <_edata>
 63e:	55                   	push   %rbp
 63f:	48 29 fe             	sub    %rdi,%rsi
 642:	48 89 e5             	mov    %rsp,%rbp
 645:	48 c1 fe 03          	sar    $0x3,%rsi
 649:	48 89 f0             	mov    %rsi,%rax
 64c:	48 c1 e8 3f          	shr    $0x3f,%rax
 650:	48 01 c6             	add    %rax,%rsi
 653:	48 d1 fe             	sar    %rsi
 656:	74 18                	je     670 <register_tm_clones+0x40>
 658:	48 8b 05 91 09 20 00 	mov    0x200991(%rip),%rax        # 200ff0 <_DYNAMIC+0x1b8>
 65f:	48 85 c0             	test   %rax,%rax
 662:	74 0c                	je     670 <register_tm_clones+0x40>
 664:	5d                   	pop    %rbp
 665:	ff e0                	jmpq   *%rax
 667:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
 66e:	00 00 
 670:	5d                   	pop    %rbp
 671:	c3                   	retq   
 672:	0f 1f 40 00          	nopl   0x0(%rax)
 676:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
 67d:	00 00 00 

0000000000000680 <__do_global_dtors_aux>:
 680:	80 3d 99 09 20 00 00 	cmpb   $0x0,0x200999(%rip)        # 201020 <_edata>
 687:	75 27                	jne    6b0 <__do_global_dtors_aux+0x30>
 689:	48 83 3d 67 09 20 00 	cmpq   $0x0,0x200967(%rip)        # 200ff8 <_DYNAMIC+0x1c0>
 690:	00 
 691:	55                   	push   %rbp
 692:	48 89 e5             	mov    %rsp,%rbp
 695:	74 0c                	je     6a3 <__do_global_dtors_aux+0x23>
 697:	48 8b 3d 7a 09 20 00 	mov    0x20097a(%rip),%rdi        # 201018 <__dso_handle>
 69e:	e8 45 ff ff ff       	callq  5e8 <_init+0x38>
 6a3:	e8 48 ff ff ff       	callq  5f0 <deregister_tm_clones>
 6a8:	5d                   	pop    %rbp
 6a9:	c6 05 70 09 20 00 01 	movb   $0x1,0x200970(%rip)        # 201020 <_edata>
 6b0:	f3 c3                	repz retq 
 6b2:	0f 1f 40 00          	nopl   0x0(%rax)
 6b6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
 6bd:	00 00 00 

00000000000006c0 <frame_dummy>:
 6c0:	48 8d 3d 69 07 20 00 	lea    0x200769(%rip),%rdi        # 200e30 <__JCR_END__>
 6c7:	48 83 3f 00          	cmpq   $0x0,(%rdi)
 6cb:	75 0b                	jne    6d8 <frame_dummy+0x18>
 6cd:	e9 5e ff ff ff       	jmpq   630 <register_tm_clones>
 6d2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 6d8:	48 8b 05 09 09 20 00 	mov    0x200909(%rip),%rax        # 200fe8 <_DYNAMIC+0x1b0>
 6df:	48 85 c0             	test   %rax,%rax
 6e2:	74 e9                	je     6cd <frame_dummy+0xd>
 6e4:	55                   	push   %rbp
 6e5:	48 89 e5             	mov    %rsp,%rbp
 6e8:	ff d0                	callq  *%rax
 6ea:	5d                   	pop    %rbp
 6eb:	e9 40 ff ff ff       	jmpq   630 <register_tm_clones>

00000000000006f0 <do_something>:
 6f0:	48 8b 15 e9 08 20 00 	mov    0x2008e9(%rip),%rdx        # 200fe0 <_DYNAMIC+0x1a8>
 6f7:	48 8b 0d d2 08 20 00 	mov    0x2008d2(%rip),%rcx        # 200fd0 <_DYNAMIC+0x198>
 6fe:	8b 02                	mov    (%rdx),%eax
 700:	89 3a                	mov    %edi,(%rdx)
 702:	89 01                	mov    %eax,(%rcx)
 704:	c3                   	retq   
 705:	90                   	nop
 706:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
 70d:	00 00 00 

0000000000000710 <do_something_else>:
 710:	89 f8                	mov    %edi,%eax
 712:	c3                   	retq   

Disassembly of section .fini:

0000000000000714 <_fini>:
 714:	48 83 ec 08          	sub    $0x8,%rsp
 718:	48 83 c4 08          	add    $0x8,%rsp
 71c:	c3                   	retq   
