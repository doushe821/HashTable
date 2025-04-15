global strcmp64byte

strcmp64byte:
    xor rax, rax
    mov rcx, 8h

ComparingCycle:

    ; rdi - first string, rsi - second string
    mov r8, [rsi]
    mov r9, [rdi]

    cmp r9, r8 
    jne UnEqualStrings

    add rsi, 8h 
    add rdi, 8h

loop ComparingCycle

    ret

UnEqualStrings:
    inc rax

    ret