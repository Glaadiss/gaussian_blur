;RDI width
;RDI+4 height
;RDI+8 r

section .text

global _sumuj

segment .text
_sumuj:
	mov rax, [rdi]     ; RAX=b
	add rax, rsi        ; RAX=RAX+a 
	ret		            ; koniec funkcji

