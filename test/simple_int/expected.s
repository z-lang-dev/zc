    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+fmt]
    mov rsi, 41
    call printf
    xor rax, rax
    pop rbp
    ret
fmt:
    .asciz "%lld\n"
