    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+ct0]
    call printf
    mov rax, 2
    push rax
    mov rax, 5
    imul rax, 3
    push rax
    pop rdi
    pop rax
    add rax, rdi
    pop rbp
    xor rax, rax
    ret
ct0:
    .asciz "Hello\n"
