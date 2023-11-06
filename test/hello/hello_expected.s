    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+fmt]
    call printf
    pop rbp
    xor rax, rax
    ret
fmt:
    .asciz "Hello, world!"
