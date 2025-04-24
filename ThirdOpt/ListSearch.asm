global ListSearch

ListSearch:
    xor rax, rax
    cmp rdx, 0h
    je .KeyFound 
    add rsi, 20h
 
.SearchLoop:

    inc rax
    vmovaps ymm0, yword [rdi]
    vmovaps ymm1, yword [rsi]

    vpxor ymm0, ymm1 
    vptest ymm0, ymm0
    jz .KeyFound

    add rsi, 20h
    sub rdx, 20h

    cmp rdx, 0h
    jne .SearchLoop

    xor rax, rax 
    ret
    

.KeyFound:
    ret
