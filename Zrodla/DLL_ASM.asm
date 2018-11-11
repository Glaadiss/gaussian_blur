;RDI data to save in
;RSI heigh
;RDX width
;Rcx new data


; function gaussBlur_1 (scl, tcl, w, h, r) {
;     var rs = Math.ceil(r * 2.57);     // significant radius
;     for(var i=0; i<h; i++)
;         for(var j=0; j<w; j++) {
;             var val = 0, wsum = 0;
;             for(var iy = i-rs; iy<i+rs+1; iy++)
;                 for(var ix = j-rs; ix<j+rs+1; ix++) {
;                     var x = Math.min(w-1, Math.max(0, ix));
;                     var y = Math.min(h-1, Math.max(0, iy));
;                     var dsq = (ix-j)*(ix-j)+(iy-i)*(iy-i);
;                     var wght = Math.exp( -dsq / (2*r*r) ) / (Math.PI*2*r*r);
;                     val += scl[y*w+x] * wght;  wsum += wght;
;                 }
;             tcl[i*w+j] = Math.round(val/wsum);            
;         }
; }


section .data
	digit equ 255
	next equ 8
	zero equ 0
	three equ 3
	one equ 1
	five equ 5
	radius equ 5
	blur equ 4
	box  equ 9
	ones times 3 db 1


default rel 
section .text

global _sumuj

segment .text
_sumuj:
	xor r10, r10 ; zerowanie r10	

	loopInColumn:  
	; mov rax, [rdi + r10] ; pierwszy pixel wiersza do rax
	xor r11, r11 ; zerowanie r11

	; mul r8, rdx
	mov rax, [rdi + r10] ; n'ty wiersz
	movaps xmm0, [rax] ; pierwszy pixel wiersza
	; movaps xmm1, [rax + rdx - 6] ; ostatni pixel wiersza 
	movaps xmm2, [ones]

	; mov 
	; mov rax, 
	; movaps xmm1, [rax + ] ; ostatni pixel wiersza
	; movaps xmm1, [rd]

	loopInRow:
	;------------------------------------------------------------------------------------
	; xor r14, r14;
	; loopInBox:

	; mov [r16], []
	; mov [r15], [r14]

	; inc r14
	; cmp r14, blur



	; jle loopInBox
	;------------------------------------------------------------------------------------


	mov byte [rax + r11], digit ;  r
	mov byte [rax + r11 + 1], digit ; g
	mov byte [rax + r11 + 2], digit ; b



	add r11, 3 ;nastepny pixel 
	cmp r11, rdx
	jl loopInRow
		
		add r10, next ; nastepny wiersz +8
		cmp r10, rsi
		jl loopInColumn
	ret		            ; koniec funkcji
