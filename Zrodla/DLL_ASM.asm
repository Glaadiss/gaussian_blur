;r15 data to save in
;RSI heigh
;RDX width
;Rcx new data


; extern void box_blur_h(Image *source, Image *target, int w, int h, int radius)
; {
;     Avg *avg = malloc(sizeof(Avg));
;     double iarr = (double)1 / (radius + radius + 1); //
;     for (int i = 0; i < h; i++)
;     {
;         int li = 0;
;         int ri = radius;
;         Pixel left_pixel = source->data[i][0];
;         Pixel right_pixel = source->data[i][w - 1];
;         init(avg, &left_pixel);
;         mulTo(avg, radius + 1);
;         for (int j = 0; j < radius; j++)
;         {
;             Pixel pixel = source->data[i][j];
;             addToP(avg, &pixel);
;         }
;         for (int j = 0; j < w; j++)
;         {
;             Pixel pixel = source->data[i][ri % w];
;             Pixel second_pixle = source->data[i][li % w];
;             if (j <= radius)
;             {
;                 ri++;
;                 addTo(avg, sub(&pixel, &left_pixel));
;             }
;             else if (j < w - radius)
;             {
;                 ri++;
;                 li++;
;                 addTo(avg, sub(&pixel, &second_pixle));
;             }
;             else
;             {
;                 li++;
;                 addTo(avg, sub(&right_pixel, &pixel));
;             }

;             Pixel result = {avg->r * iarr, avg->g * iarr, avg->b * iarr};
;             target->data[i][j] = result;
;         }
;     }
; }




section .text

global _sumuj


segment .text
default rel ; enable rip-relative addressing

_sumuj:
	; init zeros
	xor r8, r8 ; zerowanie r8 - i

	loopInColumn:  
	xor r9, r9 ; zerowanie r9 - j

	call getFirstAndLastPix ; xmm0 = left first Pix, xmm1 = right first Pix


	ret
	movdqa xmm0, [rax] ; pierwszy pixel wiersza
	movdqa [rax], xmm0
	



	
	call arrayOne
	; movdqa xmm0, [array]
	paddq xmm0, xmm0
	paddq xmm0, xmm0
	movdqa [rax], xmm0


	movdqa xmm3, xmm0 ; mov left first pix to xmm3 (avg variable)
	call arrayBlur	; initialize array with "blur"
	movdqa xmm4, [array] ; blur value to each cell (radius variable)
	call arrayOne ; initalize array with "one"
	paddq xmm4, [array] ; add one to each cell (radius variable)
	pmuldq xmm3, xmm4 ; multiple avg by (blur + 1) and save in avg
	


	mov rax, [rdi + r8 ] ; point to first pixel in row
	xor r10, r10 ; 0 to r10
	prepareAvgLoop:
		mov r11, [rax + r10] ; next pixel in row #1
		mov [rax], r11 ; next pixel in row #2
		paddq xmm3, [rax] ; add pixel values to avg values
	
	add r10, 3	; next pixel 
	cmp r10, blur
	jl prepareAvgLoop




	; paddq xmm4, [ones]

	;---------------------TEST-------
	; mov byte[blurs], blur
	; mov byte[blurs + 1], blur
	; mov byte[blurs + 2], blur
	; addpd xmm4, [blurs]
	; movdqa [rax], xmm4
	; ret
	; mov rax, [ones]
	; paddq xmm3, oword[blurs]

	
	;--------------------------------

	; movdqa [rax], xmm1 

	;paddq 
	;movdqa

	loopInRow:
	;------------------------------------------------------------------------------------
	; xor r14, r14;
	; loopInBox:

	; mov [r16], []
	; mov [r10], [r14]

	; inc r14
	; cmp r14, blur



	; jle loopInBox
	;------------------------------------------------------------------------------------


	mov byte [rax + r9], digit ;  r
	mov byte [rax + r9 + 1], digit ; g
	mov byte [rax + r9 + 2], digit ; b



	add r9, 3 ;nastepny pixel 
	cmp r9, rdx
	jl loopInRow
		
		add r8, next ; nastepny wiersz +8
		cmp r8, rsi
		jl loopInColumn
	ret		            ; koniec funkcji

;----------------------------------------------------------------------------------------------
getFirstAndLastPix:

	mov rax, [rdi + r8] ; n'ty wiersz
	movdqa xmm0, [rax] ; pierwszy pixel wiersza
	mov r11, [rax + rdx - 12] ; ostatni pixel wiersza #1 !!!!!!!!!!!!!!!!!!!
	; movdqa xmm1, [r11]
	; movdqa [r12], xmm1
	; mov [rax], r10 ; ostatni pixel wiersza #2
	; mov [r11], r10
	; movdqa xmm1, [r11] ; ostatni pixel wiersza #3
	; movdqa [r11], xmm0
	mov rax, r11
	; mov rax, r11
	ret

arrayZero:
	mov dword[array], zero
	mov dword[array +1*4], zero
	mov dword[array +2*4], zero
	ret

arrayBlur:
	mov dword[array], blur
	mov dword[array +1*4], blur
	mov dword[array +2*4], blur
	ret

arrayOne:
	mov dword[array], one
	mov dword[array +1*4], one
	mov dword[array +2*4], one
	ret

section .data
	array db 0, 0, 0
	digit equ 255
	nextPix equ 24
	next equ 8
	zero equ 0
	three equ 3
	one equ 1
	five equ 5
	radius equ 5
	blur equ 1
	box  equ 9
	ones dw 1, 1, 1

; section .bss
; 	zeros: resb 64
