; global ListSearch

; ListSearch:
;     xor rax, rax
;     cmp rdx, 0h
;     je .KeyFound

;     add rsi, 20h
 
; .SearchLoop:

;     inc rax
;     vmovaps ymm0, yword [rdi]
;     vmovaps ymm1, yword [rsi]

;     vpxor ymm0, ymm1 
;     vptest ymm0, ymm0
;     jz .KeyFound

;     add rsi, 20h
;     sub rdx, 20h

;     cmp rdx, 0h
;     jne .SearchLoop

;     xor rax, rax 
;     ret
    

; .KeyFound:
;     ret

global ListSearch

ListSearch:
    xor rax, rax
    cmp rdx, 0h
    vmovaps ymm0, yword [rdi]
    vmovaps ymm2, yword [rsi]
    je .KeyFound

    add rsi, 20h
 
.SearchLoop:

    inc rax
    add rsi, 20h

    vmovaps ymm1, ymm2
    vmovaps ymm2, yword [rsi]
    vpxor ymm3, ymm1, ymm0 
    vptest ymm3, ymm3
    jz .KeyFound

    sub rdx, 20h

    cmp rdx, 0h
    jne .SearchLoop

    xor rax, rax 
    ret
    

.KeyFound:
    ret
