global strcmp64byte

strcmp64byte:

    mov [oldRSP], rsp
    pop rax 
    mov [lostIP], rax

    mov rcx, 8h

ComparingCycle:


    ; rdi - first string, rsi - second string
    mov rax, [rsi]
    mov rbx, [rdi]

    cmp rax, rbx 
    jne UnEqualStrings

    add rsi, 8h 
    add rdi, 8h

loop ComparingCycle

; equalStrings:
    mov rax, [lostIP]
    mov r8, [oldRSP]
    mov [r8], rax 
    mov rsp, [oldRSP]

    mov rax, 00h

    ret

UnEqualStrings:

    mov rax, [lostIP]
    mov r8, [oldRSP]
    mov [r8], rax 
    mov rsp, [oldRSP]

    mov rax, 1h

    ret


section .data

oldRSP dq 0x00
lostIP dq 0x00
