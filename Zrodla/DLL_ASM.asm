;RDI data
;RSI bytes to go through
;RDX width


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
	digit equ 42
	zero equ 0
	one equ 1
	five equ 5
	radius equ 5

section .text

global _sumuj

segment .text
_sumuj:
	mov rax, zero
	
	
	loop:
	mov byte [rdi + rax], digit
	inc rax
	cmp rax, rsi	

	jne loop


	mov rax, rdi      ; RAX=RAX+a 
	ret		            ; koniec funkcji
