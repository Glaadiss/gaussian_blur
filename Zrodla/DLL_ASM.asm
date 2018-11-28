
;rdi data to save in
;RSI heigh
;RDX width
;Rcx new data
;r8 radius

default rel 						; enable rip-relative addressing
; section .bss
; blurI: resb 4


section .data
	blurArr dd 0, 0, 0, 0			; blur level buffer
	ones dd 1, 1, 1, 1				; array with ones
	blurI dd 0, 0, 0, 0				; blur level
	zeros dd 0, 0, 0, 0				; array with zeros
	blurBuff dd 0, 0 ,0 ,0			; increased array with blur level

	next equ 8						; for moving between pointers
	zero equ 0						;	 
	one equ 1						;


section .text

global _gaussian_blur
section .text

_gaussian_blur:
	mov eax, r8d					; mov radius to eax
	mov dword[blurBuff], eax		; fill blurBuff with radius
	add dword[blurBuff], eax		; multiply blurBuff x2
	add dword[blurBuff], 1			; add 1 to blurBuff
	mov dword[blurArr], eax			; fill BlurArr first element with blurr
	mov dword[blurArr + 4], eax		; fill BlurArr second element with blurr
	mov dword[blurArr + 8], eax		; fill BlurArr third element with blurr
	imul eax, 8						; multiple eax x8
	mov dword[blurI], eax			; fill blurI with x8 radius

	xor r8, r8						; zero to r8
	heightLoop:
	
	movdqa xmm0, [zeros]
	; 
	mov rax, [rdi + r8] 			; n row
	mov r12, [rax]
	movdqa xmm1, [r12] 				; left_pix
	mov r12, [rax + rdx - 8]
	movdqa xmm2, [r12]	
	movdqa xmm0, xmm1 				; mov left first pix to xmm0 (avg variable)
	movdqa xmm4, [blurArr] 			; blur value to each cell (radius variable)
	paddq xmm4, [ones] 				; add one to each cell (radius variable)
	pmullw xmm0, xmm4 				; multiple avg by (blur + 1) and save in avg
	
	;----------------first looop -----------------;
	mov rax, [rdi + r8 ] 			; point to first pixel in row
	xor r10, r10					; 0 to r10
	avgLoop:
		mov r13, [rax + r10] 		; next pixel in row #
		movdqa xmm2, [r13]			; mov pixel to first right pixel
		paddq xmm0, xmm2			; add first right pixel to avg
		add r10, next				; next pixel 
		cmp r10, [blurI]			; if r10 < radius
		jl avgLoop
	;----------------first looop -----------------;

	;----------------second looop -----------------;	
	mov rax, [rdi + r8 ] 			; point to first pixel in row
	mov r10, 0 						; loop index
    mov r15, [blurI]				; ri
	mov rbx, 0	
	mainLoop:
		mov r13, [rax + r15]	
		movdqa xmm3, [r13] 			; first_pixel	
		add rax, rbx
		mov r13, [rax]
		movdqa xmm4, [r13] 			; second_pixel
		
		;-------- jumps conditions --------
		cmp r10, [blurI]			; 
		jle lessThanRadius			; if widthIndex < blurValue
		mov r11, rdx				; tmp value for width
		sub r11, [blurI]			; tmp = width - blurValue
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
		add rbx, next				; increment li												 				
		add r15, next 				; increment ri
		movdqa xmm5, xmm3			; temp vector - from first_pixel
		psubd xmm5, xmm4			; tmp = first_pixel - second_pixel 
		paddd xmm0, xmm5			; add tmp to avg
		jmp mainComp

		lessThanWidth:
		add rbx, next					
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
		mov r13, [blurBuff]			; move blur value to r13
		div r13 					; r / blur value
		mov dword[r9], eax			; save new r value

		mov eax, [r12+4]			; move g from pixel to eax
		mov edx, 0					; move 0 to edx to work only on integers
		mov r13, [blurBuff]			; move blur value to r13
		div r13 					; g / blur value
		mov dword[r9+4], eax		; save new g value

		mov eax, [r12+8]			; move b from pixel to eax
		mov edx, 0					; move 0 to edx to work only on integers
		mov r13, [blurBuff]				; move blur value to r13
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
		ret
