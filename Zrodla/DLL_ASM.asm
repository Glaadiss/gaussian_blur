section .text

global sumuj
segment .text
sumuj:

	mov rax, rdi        ; RAX=b
	add rax, rsi        ; RAX=RAX+a 
	ret		            ; koniec funkcji
                        ; zwracana wartość funkcji przekazywana jest do programu poprzez rejestr RAX

section .data
napis db "<<<FUNCJA SUMUJ>>>",0Ah
offset equ $ - napis