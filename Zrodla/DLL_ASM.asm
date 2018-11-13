;rdi data to save in
;RSI heigh
;RDX width
;Rcx new data


section .text

global _sumuj
segment .text
default rel ; enable rip-relative addressing

_sumuj:
	; init zeros
	xor r8, r8 ; zerowanie r8 - i
	heightLoop:
	call arrayZero
	movdqa xmm0, [array]



	call getFirstAndLastPix ; xmm0 = left first Pix, xmm1 = right first Pix

	movdqa xmm0, xmm1 ; mov left first pix to xmm0 (avg variable)
	call arrayBlur	; initialize array with "blur"
	movdqa xmm4, [array] ; blur value to each cell (radius variable)
	call arrayOne ; initalize array with "one"
	paddq xmm4, [array] ; add one to each cell (radius variable)
	pmullw xmm0, xmm4 ; multiple avg by (blur + 1) and save in avg


	;----------------first looop -----------------;
	mov rax, [rdi + r8 ] ; point to first pixel in row
	xor r10, r10 ; 0 to r10

	avgLoop:

		mov r13, [rax + r10] ; next pixel in row #
		movdqa xmm2, [r13]
		paddq xmm0, xmm2	
		add r10, next	; next pixel 
		cmp r10, blur*8
		jl avgLoop

	;----------------first looop -----------------;


	;----------------second looop -----------------;
	mov rax, [rdi + r8 ] ; point to first pixel in row
	xor r10, r10 ; loop index
	xor r14, r14 ; li
	mov r15, blur*8; ri 
	mainLoop:
		mov r13, [rax + r15]	
		movdqa xmm3, [r13] ; first_pixel
		
		mov r13, [rax + r14]
		movdqa xmm4, [r13] ; second_pixel
		; movdqa xmm4, [array]
		
		;-------- jumps conditions --------
		cmp r10, blur*8
		jle lessThanRadius
		mov r11, rdx
		sub r11, blur*8

		cmp r10, r11
		jl lessThanWidthMinusRadius
		jmp lessThanWidth
		;-------- jumps conditions --------

		lessThanRadius:
		add r15, next ; increment ri
		movdqa xmm5, xmm3
		psubq xmm5, xmm1
		paddq xmm0, xmm5
		jmp mainComp

		lessThanWidthMinusRadius:
		
		add r14, next ; increment li
		add r15, next ; increment ri
		movdqa xmm5, xmm3
		psubd xmm5, xmm4
		paddd xmm0, xmm5
		cmp r10, 2000
		jg end			
		jmp mainComp

		lessThanWidth:
		add r14, next ; increment li
		movdqa xmm5, xmm2
		psubq xmm5, xmm3
		paddq xmm0, xmm5
;                 addTo(avg, sub(&right_pixel, &pixel));

		mainComp:
		

		; movdqa xmm6, xmm0		
		call arrayFives
		; vdivpd xmm6, [array]
		movdqa [r12], xmm0
		; movdqa [r12], xmm6

		; mov ebp, eax
		; mov r13, [eax]
		mov rax, [rcx + r8]
		mov r9, [rax + r10]

		mov r11, rdx

		mov eax, [r12]
		mov edx, 0
		mov r13, 11
		div r13 		
		mov dword[r9], eax

		mov eax, [r12+4]
		mov edx, 0
		mov r13, 11
		div r13 		
		mov dword[r9+4], eax

		mov eax, [r12+8]
		mov edx, 0
		mov r13, 11
		div r13 
		mov dword[r9+8], eax

		mov rdx, r11		

		mov rax, [rdi + r8]

		add r10, next
		cmp r10, rdx
		jl mainLoop

		
		end: 
		add r8, next
		cmp r8, rsi
		jl heightLoop
		movdqa [r12], xmm0
		mov rax, r12
		ret
;----------------------------------------------------------------------------------------------
getFirstAndLastPix:

	mov rax, [rdi + r8] ; n'ty wiersz
	mov r12, [rax]
	movdqa xmm1, [r12] ; pierwszy pixel wiersza
	mov r9, rdx
	mov r12, [rax + r9 - 8]
	movdqa xmm2, [r12]
	ret

arrayZero:
	mov dword[array], zero
	mov dword[array +4], zero
	mov dword[array +8], zero
	ret

arrayBlur:
	mov dword[array], blur
	mov dword[array +4], blur
	mov dword[array +8], blur
	ret

arrayOne:
	mov dword[array], one
	mov dword[array +4], one
	mov dword[array +8], one
	ret

arrayFives:
	mov dword[array], five
	mov dword[array +4], five
	mov dword[array +8], five
	ret

section .data
	array db 0, 0, 0
	digit equ 255
	nextPix equ 24
	next equ 8
	zero equ 0
	three equ 3
	one equ 1
	five equ 11
	radius equ 5
	blur equ 5
	blurIndex dw 0
	iarr dd 0.2
	iarArr dd 0.2, 0.2, 0.2
	iar dw 2
	box equ 9
	ones dw 1, 1, 1

; section .bss
; 	zeros: resb 64
