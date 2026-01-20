MAIN:	add r3,LIST
LOOP:	prn #48
        macr m_macr
	cmp r3, #-6
	bne END
	endmacr
	lea STR, r6
	inc r6
	mov *r6,K
	sub           r1, r4
	                                         
	;


;




;;;;
	fn1
	dx2

	macr m_test
	lea STR, r6
	inc dd
	endmacr
	mov *r6,K
	sub r1, r4
	dec K
	m_test
	m_macr	
	jmp LOOP
END:	stop
STR: 	.string "abcd"
LIST: 	.data 6, -9
	.data -100
K: 	.data 31
