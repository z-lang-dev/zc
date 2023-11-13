    .intel_syntax noprefix
    .text
    .global main
main:
    mov rax, 3
    imul rax, 5
    push rax
    mov rax, 2
    neg rax
    push rax
    pop rdi
    pop rax
    add rax, rdi
    sub rax, 8
    neg rax
    ret
