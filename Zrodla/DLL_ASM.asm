
;rdi data to save in
;RSI heigh
;RDX width
;Rcx new data
;r8 radius

default rel 						; enable rip-relative addressing
section .bss
blurI: resb 4

section .data
	array db 0, 0, 0
	next equ 8
	zero equ 0
	one equ 1
	five equ 11
	radius equ 5
	blur equ 5


section .text

global _gaussian_blur
section .text

_gaussian_blur:
	mov rax, r8
	mov r9, 8
	imul rax, r9
	mov dword[blurI], eax


	; mov r9, next
	; mul r9
	; mov blur*8, rax

	; init zeros
	xor r8, r8 						; zero to r8
	heightLoop:

	call arrayZero
	movdqa xmm0, [array]
	call getFirstAndLastPix 		; xmm0 = left first Pix, xmm1 = right first Pix
	movdqa xmm0, xmm1 				; mov left first pix to xmm0 (avg variable)
	call arrayBlur					; initialize array with "blur"
	movdqa xmm4, [array] 			; blur value to each cell (radius variable)
	call arrayOne 					; initalize array with "one"
	paddq xmm4, [array] 			; add one to each cell (radius variable)
	pmullw xmm0, xmm4 				; multiple avg by (blur + 1) and save in avg

	;----------------first looop -----------------;
	mov rax, [rdi + r8 ] 			; point to first pixel in row
	xor r10, r10 					; 0 to r10
	avgLoop:

		mov r13, [rax + r10] 		; next pixel in row #
		movdqa xmm2, [r13]
		paddq xmm0, xmm2	
		add r10, next	; next pixel 
		cmp r10, blur*8
		jl avgLoop
	;----------------first looop -----------------;


	;----------------second looop -----------------;
	mov rax, [rdi + r8 ] 			; point to first pixel in row
	xor r10, r10 					; loop index
	xor r14, r14 					; li
	mov r15, blur*8					; ri 
	mainLoop:
		mov r13, [rax + r15]	
		movdqa xmm3, [r13] 			; first_pixel	
		mov r13, [rax + r14]
		movdqa xmm4, [r13] 			; second_pixel
		
		;-------- jumps conditions --------
		cmp r10, blur*8				; 
		jle lessThanRadius			; if widthIndex < blurValue
		mov r11, rdx				; tmp value for width
		sub r11, blur*8				; tmp = width - blurValue
		sub r11, 8					; tmp -= 8
		cmp r10, r11				;	
		jl lessThanWidthMinusRadius ; if widthIndex < width - blurvalue
		jmp lessThanWidth			; if widthIndex < width
		;-------- jumps conditions --------

		lessThanRadius:
		add r15, next 				; increment ri
		movdqa xmm5, xmm3			; temp vector - from first_pixel
		psubq xmm5, xmm1			; tmp = fist_pixel - left_first_pixel
		paddq xmm0, xmm5			; add tmp to avg
		jmp mainComp

		lessThanWidthMinusRadius:
		
		add r14, next 				; increment li
		add r15, next 				; increment ri
		movdqa xmm5, xmm3			; temp vector - from first_pixel
		psubd xmm5, xmm4			; tmp = first_pixel - second_pixel 
		paddd xmm0, xmm5			; add tmp to avg
		jmp mainComp

		lessThanWidth:
		add r14, next 				; increment li
		movdqa xmm5, xmm2			; temp vector - from right_pixel
		psubq xmm5, xmm4			; tmp = right pixel - second_pixel
		paddq xmm0, xmm5			; add tmp to avg

		mainComp:	
		movdqa [r12], xmm0			; move avg vector to r12
		mov rax, [rcx + r8]			; move r8 row to rax (data to save in)
		mov r9, [rax + r10]			; mow r10 col from r8 row to r9 (data to save in)

		mov r11, rdx				; temporary variable for rdx - image width
		mov eax, [r12]				; move  r from pixel to eax
		mov edx, 0					; move 0 to edx to work only on integers
		mov r13, five				; move blur value to r13
		div r13 					; r / blur value
		mov dword[r9], eax			; save new r value

		mov eax, [r12+4]			; move g from pixel to eax
		mov edx, 0					; move 0 to edx to work only on integers
		mov r13, five				; move blur value to r13
		div r13 					; g / blur value
		mov dword[r9+4], eax		; save new g value

		mov eax, [r12+8]			; move b from pixel to eax
		mov edx, 0					; move 0 to edx to work only on integers
		mov r13, five				; move blur value to r13
		div r13 					; b / blur value
		mov dword[r9+8], eax		; save new b value

		mov rdx, r11				; restore width value to rdx
		mov rax, [rdi + r8]			; set rax to point on the source data
		add r10, next				; iterate width index
		cmp r10, rdx				; compare widths 
		jl mainLoop

		end: 
		add r8, next				; iterate height index
		cmp r8, rsi					; compare heights
		jl heightLoop
		movdqa [r12], xmm0			; ONLY FOR TESTS
		mov rax, r12				; ONLY FOR TESTS
		ret							; ONLY FOR TESTS
;----------------------------------------------------------------------------------------------
getFirstAndLastPix:

	mov rax, [rdi + r8] 			; n row
	mov r12, [rax]
	movdqa xmm1, [r12] 				; left_pix
	mov r12, [rax + rdx - 8]
	movdqa xmm2, [r12]				; right_pix
	ret

arrayZero:
	mov dword[array], zero
	mov dword[array +4], zero
	mov dword[array +8], zero
	ret

arrayBlur:
	; mov eax, [blur]
	mov dword[array], blur
	mov dword[array +4], blur
	mov dword[array +8], blur
	; mov rax, [rdi + r8] 	
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




