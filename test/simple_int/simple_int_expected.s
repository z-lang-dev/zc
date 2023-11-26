    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+ct0]
    mov rsi, 41
    call printf
    pop rbp
    ret
ct0:
    .asciz "%d\n"
