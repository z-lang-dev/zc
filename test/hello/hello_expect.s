    .intel_syntax noprefix
    .text
.global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+msg]
    call puts
    pop rbp
    xor rax, rax
    ret
msg:
    .asciz "Hello, world!"
